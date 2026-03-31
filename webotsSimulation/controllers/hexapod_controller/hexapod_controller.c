#include <webots/robot.h>
#include <webots/motor.h>
#include <webots/keyboard.h>
#include <math.h>
#include <stdio.h>

#define TIME_STEP 16
#define L1 0.036
#define L2 0.0813

#define STAND_Z_START 0.0
#define STAND_Z_END -0.10
#define STAND_TIME 2.0

#define STAND_X 0.06
#define STAND_Y 0.00

#define STEP_HEIGHT 0.03
#define STEP_LENGTH 0.01
#define COAX_SWING 0.16
#define CYCLE_TIME 1.0

typedef struct {
    WbDeviceTag coax, femur, tibia;
    int side; // -1=lewa, 1=prawa
    double mount_angle;
} Leg;

Leg legs[6];

void init_leg(int index, const char* prefix, int side, float mount_angle) {
    char name[64];
    sprintf(name, "coax_%s_motor", prefix);
    legs[index].coax = wb_robot_get_device(name);
    sprintf(name, "femur_%s_motor", prefix);
    legs[index].femur = wb_robot_get_device(name);
    sprintf(name, "tibia_%s_motor", prefix);
    legs[index].tibia = wb_robot_get_device(name);
    
    wb_motor_set_velocity(legs[index].coax, 5.0);
    wb_motor_set_velocity(legs[index].femur, 5.0);
    wb_motor_set_velocity(legs[index].tibia, 5.0);
    
    legs[index].side = side;
    legs[index].mount_angle = mount_angle;
}

void ik_solve(double x, double y, double z, double *coax, double *femur, double *tibia) {
    *coax = atan2(y, x);
    double r = sqrt(x*x + y*y);
    double d = sqrt(r*r + z*z);
    
    if (d > L1 + L2) d = L1 + L2 - 1e-6;
    
    double a1 = atan2(z, r);
    double a2 = acos((L1*L1 + d*d - L2*L2) / (2*L1 *d));
    *femur = a1 + a2;
    
    double a3 = acos((L1*L1 + L2*L2 - d*d) / (2*L1*L2));
    *tibia = -(M_PI - a3);
}

void move_leg(int leg_id, double x, double y, double z) {
    double coax, femur, tibia;
    ik_solve(x, y, z, &coax, &femur, &tibia);
    
    if(legs[leg_id].side == -1) coax = -coax;
    
    tibia = -tibia;
    
    wb_motor_set_position(legs[leg_id].coax, coax);
    wb_motor_set_position(legs[leg_id].femur, femur);
    wb_motor_set_position(legs[leg_id].tibia, tibia);
}

void move_leg_robot_coords(int leg_id, double x_rob, double y_rob, double z_rob) {
    double beta = legs[leg_id].mount_angle;
    
    double x_loc = x_rob * cos(beta) + y_rob * sin(beta);
    double y_loc = -x_rob * sin(beta) + y_rob * cos(beta);
    
    double coax, femur, tibia;
    ik_solve(x_loc, y_loc, z_rob, &coax, &femur, &tibia);
    
    if(legs[leg_id].side == -1) coax = -coax;
    tibia = -tibia;
    
    wb_motor_set_position(legs[leg_id].coax, coax);
    wb_motor_set_position(legs[leg_id].femur, femur);
    wb_motor_set_position(legs[leg_id].tibia, tibia);
}

void set_leg_angles(int leg_id, double coax_angle, double femur_angle, double tibia_angle) {
    double final_coax = legs[leg_id].mount_angle + coax_angle;
    if(legs[leg_id].side == -1)
        final_coax = -final_coax;
    
    wb_motor_set_position(legs[leg_id].coax, final_coax);  // ← było coax_angle
    wb_motor_set_position(legs[leg_id].femur, femur_angle);
    wb_motor_set_position(legs[leg_id].tibia, tibia_angle);
}


void stand_up() {
    double start_time = wb_robot_get_time();

    while(wb_robot_step(TIME_STEP) != -1) {
        double t = wb_robot_get_time() - start_time;
        if(t > STAND_TIME) break;
        
        double rato = t / STAND_TIME;
        double z = STAND_Z_START + rato * (STAND_Z_END - STAND_Z_START);
        
        for(int i=0; i<6; i++) {
            move_leg(i, STAND_X, STAND_Y, z);
        }
    }
}

void get_stand_angles(double *base_coax, double *base_femur, double *base_tibia) {
    ik_solve(STAND_X, STAND_Y, STAND_Z_END, base_coax, base_femur, base_tibia);
    *base_tibia = -(*base_tibia);
}

//direction -1 = przod, 1 = tyl
void walk(double time, double direction) {
    double freq = 2.0 * M_PI / CYCLE_TIME;
    double phase = fmod(time * freq, 2.0 * M_PI);
    
    for(int i = 0; i < 6; i++) {
        int is_group_a = (i == 0 || i == 2 || i == 4);
        
        double x_offset = STAND_X;
        double y_offset = 0.0;
        double z_offset = STAND_Z_END;
        
        double current_phase = is_group_a ? phase : fmod(phase + M_PI, 2.0 * M_PI);

        if(current_phase < M_PI) {
            double swing = current_phase / M_PI;
            
            y_offset = (-STEP_LENGTH / 2.0) - (swing * STEP_LENGTH);
            y_offset *= direction;
            
            double lift = sin(current_phase);
            z_offset += STEP_HEIGHT * lift;
        } 
        else {
            double stance = (current_phase - M_PI) / M_PI;
            
            y_offset = (STEP_LENGTH / 2.0) + (stance * STEP_LENGTH);
            y_offset *= direction;
        }
        
        move_leg_robot_coords(i, x_offset, y_offset, z_offset);
    }
}

void rotate(double time, int direction) {  // -1 = lewo, 1 = prawo
    double freq = 2.0 * M_PI / CYCLE_TIME;
    double phase = fmod(time * freq, 2.0 * M_PI);
    
    double base_coax, base_femur, base_tibia;
    get_stand_angles(&base_coax, &base_femur, &base_tibia);
    
    for(int i=0; i<6; i++) {
        int is_group_a = i == 0 || i == 2 || i == 4;
        int side = legs[i].side;
        
        double coax_offset = 0.0;
        double femur_offset = 0.0;
        double tibia_offset = 0.0;
        
        double coax_dir = -side * direction;
        
        if(is_group_a) {
            if(phase < M_PI) {
                double swing_progress = phase / M_PI;
                coax_offset = COAX_SWING * (-1.0 + 2.0 * swing_progress) * coax_dir;
                
                double lift = 4.0 * swing_progress * (1.0 - swing_progress);
                femur_offset = 0.15 * lift;
                tibia_offset = 0.1 * lift;
            } 
            else {
                double stance_progress = (phase - M_PI) / M_PI;
                coax_offset = COAX_SWING * (1.0 - 2.0 * stance_progress) * coax_dir;
            }
        } 
        else {
            if(phase >= M_PI) {
                double swing_progress = (phase - M_PI) / M_PI;
                coax_offset = COAX_SWING * (-1.0 + 2.0 * swing_progress) * coax_dir;
                
                double lift = 4.0 * swing_progress * (1.0 - swing_progress);
                femur_offset = 0.15 * lift;
                tibia_offset = 0.1 * lift;
            } else {
                double stance_progress = phase / M_PI;
                coax_offset = COAX_SWING * (1.0 - 2.0 * stance_progress) * coax_dir;
            }
        }
        
        set_leg_angles(i, base_coax + coax_offset, 
                       base_femur + femur_offset, 
                       base_tibia + tibia_offset);
    }
}

void control_loop() {
    double walk_time = 0.0;
    double last_time = wb_robot_get_time();
    
    while(wb_robot_step(TIME_STEP) != -1) {
        double current_time = wb_robot_get_time();
        double dt = current_time - last_time;
        last_time = current_time;
        
        int key = wb_keyboard_get_key();
        
        double forward = 0.0;
        int rotation_dir = 0;
        
        
        
        // double test_angle = 0.4 * sin(current_time * 25.0);
        // set_leg_angles(5, test_angle, 0, 0);
        
        
        if(key == 'W') {
            forward = -1.0;
        } 
        else if(key == 'S') {
            forward = 1.0;
        } 
        else if(key == 'D') {
            rotation_dir = 1;
        } 
        else if(key == 'A') {
            rotation_dir = -1;
        }
        
        if(rotation_dir != 0) {
            rotate(current_time, rotation_dir);
        } 
        else if(forward != 0.0) {
            walk_time += dt;
            walk(walk_time, forward);
        } 
        else {
            for(int i=0; i<6; i++) {
                move_leg(i, STAND_X, STAND_Y, STAND_Z_END);
            }
            walk_time = 0.0;
        }
    }
}

int main() {
    wb_robot_init();
    
    init_leg(0, "lf", -1, 0.896);
    init_leg(1, "lm", -1, 0.0);
    init_leg(2, "lb", -1, -0.896);
    init_leg(3, "rf", 1, 0.896);
    init_leg(4, "rm", 1, 0.0);
    init_leg(5, "rb", 1, -0.896);
    
    wb_keyboard_enable(TIME_STEP);
    
    stand_up();
    
    control_loop();
    
    wb_robot_cleanup();
    return 0;
}