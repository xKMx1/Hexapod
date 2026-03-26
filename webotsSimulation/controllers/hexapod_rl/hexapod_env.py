import numpy as np
import gymnasium as gym
from gymnasium import spaces

from controller import Supervisor #, Motor, IntertialUnit, GPS, Gyro

TIME_STEP = 32                 # ms but it doesnt work precisly
MAX_EPISODE_STEPS = 1000

JOINT_LIMITS = {
    "coax":  (-0.1, 0.1),      # from webots
    "femur": (-1.0,  1.0),              # temp values, to be checked
    "tibia": (-1.0,  2),
}

LEG_PREFIXES = ["lf", "lm", 'lb', "rf", "rm", "rb"]
LEG_SIDES = [-1, -1, -1, 1, 1, 1]           # left side = -1, right side = 1

BASE_POSE = [0.0, -0.294316, 1.36397]

class HexapodEnv(gym.Env):
    '''
    Observation space:
    [0, 2]   -> GPS postion          (x,y,z)
    [3, 5]   -> IMU                  (roll, pitch, yaw)
    [6, 8]   -> linear velocity      (vx, vy, vz)
    [9, 11]  -> angular valocity     (wx, wy, wz)
    [12, 29] -> servos angles        (18x[-1, 1])
    [30, 47] -> last state of servos (18x[-1, 1])

    Action space:
    angle for each servo (18x[-1, 1])
    '''


    def __init__(self, render_mode=None):
        super().__init__()

        self.render_mode = render_mode
        self._step_count = 0
        self._prev_position = np.zeros(3)
        self._prev_action = np.zeros(18)

        

try:
    robot = Supervisor()
    timestep = int(TIME_STEP)
    print(f"dziala {int(robot.getBasicTimeStep())}")

    if robot.step(timestep) != -1:
        print("symulaja chodzi")

    gps = robot.getDevice("gps")
    gps.enable(timestep)

    imu = robot.getDevice("inertial unit")
    imu.enable(timestep)

    gyro = robot.getDevice("gyro")
    gyro.enable(timestep)

    motor = robot.getDevice("femur_lf_motor")
    motor.setPosition(0.5)

    while robot.step(timestep) != -1:
        values_gps = gps.getValues()
        values_gyro = gyro.getValues()
        values_imu = imu.getRollPitchYaw()

        gps_fmt  = [f"{v:8.3f}" for v in values_gps]
        imu_fmt  = [f"{v:8.3f}" for v in values_imu]
        gyro_fmt = [f"{v:8.3f}" for v in values_gyro]

        print(f"GPS: {gps_fmt}  IMU: {imu_fmt}  Gyro: {gyro_fmt}")

except Exception as e: 
    print(f"Błąd {e}")