#include <webots/robot.h>
#include <webots/motor.h>
#include <webots/keyboard.h>
#include <math.h>
#include <stdio.h>

#define TIME_STEP 16
#define L1 0.07
#define L2 0.08

#define STAND_Z_START 0.0
#define STAND_Z_END -0.10
#define STAND_TIME 2.0
#define STAND_X 0.06
#define STAND_Y 0.00

#define STEP_HEIGHT 0.05
#define STEP_LENGTH 0.03
#define COAX_SWING 0.16
#define CYCLE_TIME 0.8

typedef struct {
    WbDeviceTag coax, femur, tibia;
    int side; // -1=lewa, 1=prawa
} Leg;

Leg legs[6];

void init_leg(int index, const char* prefix, int side) {
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

void set_leg_angles(int leg_id, double coax_angle, double femur_angle, double tibia_angle) {
    if(legs[leg_id].side == -1)
        coax_angle = -coax_angle;
    
    wb_motor_set_position(legs[leg_id].coax, coax_angle);
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
    
    double base_coax, base_femur, base_tibia;
    get_stand_angles(&base_coax, &base_femur, &base_tibia);
    
    for(int i = 0; i < 6; i++) {
        int is_group_a = i == 0 || i == 2 || i == 4;
        
        double coax_offset = 0.0;
        double femur_offset = 0.0;
        double tibia_offset = 0.0;
        
        if(is_group_a) {
            if(phase < M_PI) {
                // noga w gorze
                double swing_progress = phase / M_PI;
                
                coax_offset = COAX_SWING * sin(M_PI * swing_progress) * direction;

                double lift = 4.0 * swing_progress * (1.0 - swing_progress);
                femur_offset = 0.15 * lift;
                tibia_offset = 0.1 * lift;
            } 
            else {
                // noga na ziemi
                double stance_progress = (phase - M_PI) / M_PI;
                coax_offset = COAX_SWING * (1.0 - stance_progress) * direction * (-1.0);
            }
        } 
        else {
            if(phase >= M_PI) {
                double swing_progress = (phase - M_PI) / M_PI;
                coax_offset = COAX_SWING * sin(M_PI * swing_progress) * direction;
                
                double lift = 4.0 * swing_progress * (1.0 - swing_progress);
                femur_offset = 0.15 * lift;
                tibia_offset = 0.1 * lift;
            } 
            else {
                double stance_progress = phase / M_PI;
                coax_offset = COAX_SWING * (1.0 - stance_progress) * direction * (-1.0);
            }
        }
        
        set_leg_angles(i, base_coax + coax_offset, 
                       base_femur + femur_offset,
                       base_tibia + tibia_offset);
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
    
    init_leg(0, "lf", -1);
    init_leg(1, "lm", -1);
    init_leg(2, "lb", -1);
    init_leg(3, "rf", 1);
    init_leg(4, "rm", 1);
    init_leg(5, "rb", 1);
    
    wb_keyboard_enable(TIME_STEP);
    
    stand_up();
    
    control_loop();
    
    wb_robot_cleanup();
    return 0;
}