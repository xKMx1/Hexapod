import gymnasium as gym
from gymnasium import spaces

from controller import Supervisor, Motor, IntertialUnit, GPS, Gyro


TIME_STEP = 32                  # ms
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


    def __init__(self):
        super.__init__()

        