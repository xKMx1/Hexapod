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

FALL_THRESHOLD_RAD = 0.8
MIN_HEIGHT = 0.04

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
    angle for each servo normalized (18x[-1, 1]) 
    '''


    def __init__(self, render_mode=None):
        super().__init__()

        self.render_mode = render_mode
        self._step_count = 0
        self._prev_position = np.zeros(3)
        self._prev_action = np.zeros(18)

        self.joint_types = ["coax", "femur", "tibia"]

        # robot

        self.robot = Supervisor()
        self.timestep = TIME_STEP

        # sensors

        self.sensors = []   # sensor = motor's positional sensor/encoder

        self.imu = self.robot.getDevice("inertial unit")
        self.gps = self.robot.getDevice("gps")
        self.gyro = self.robot.getDevice("gyro")

        self.imu.enable(TIME_STEP)
        self.gps.enable(TIME_STEP)
        self.gyro.enable(TIME_STEP)

        # motors

        self.motors = []
        
        for prefix in LEG_PREFIXES:
            for joint_type in self.joint_types:
                motor_name = f"{joint_type}_{prefix}_motor"
                sensor_name = f"{joint_type}_{prefix}_sensor"

                m = self.robot.getDevice(motor_name)
                s = self.robot.getDevice(sensor_name)

                m.setVelocity(3.0)
                s.enable(TIME_STEP)

                self.motors.append(m)
                self.sensors.append(s)

        # action and observation space

        self.action_space = spaces.Box(
            low=-1.0, high=1.0,
            shape=(18,), dtype=np.float32
        )

        obs_dim = 48
        self.observation_space = spaces.Box(
            low=-np.inf, high=np.inf,
            shape=(obs_dim,), dtype=np.float32
        )

        self._action_scales = self._build_action_scales()

    def _build_action_scales(self):
        scales = []

        for i in range(6):
            l_coax, h_coax = JOINT_LIMITS["coax"]
            l_femur, h_femur = JOINT_LIMITS["femur"]
            l_tibia, h_tibia = JOINT_LIMITS["tibia"]

            scales += [
                (h_coax - l_coax) / 2.0,
                (h_femur - l_femur) / 2.0,
                (h_tibia - l_tibia) / 2.0,
            ]
        
        return np.array(scales, dtype=np.float32)

    def _get_observation(self):
        pos = np.array(self.gps.getValues(), dtype=np.float32)
        rot = np.array(self.imu.getRollPitchYaw(), dtype=np.float32)
        gyro = np.array(self.gyro.getValues(), dtype=np.float32)

        vel = (pos - self._prev_position) / (TIME_STEP * 1e-3)
        self._prev_position = pos.copy()

        joint_angles = np.array(
            [s.getValue() for s in self.sensors], dtype=np.float32
        )
        joints_normalized = self._normalize_joints(joint_angles)

        observation = np.concatenate([
            pos, rot, vel, gyro, joints_normalized, self._prev_action
        ])
        return observation.astype(np.float32)


    def _normalize_joints(self, angles):
        normalized = np.zeros(18, dtype=np.float32)

        for i in range(6):
            for joint_index, joint_type in enumerate(self.joint_types):
                motor_i = i * 3 + joint_index
                low, high = JOINT_LIMITS[joint_type]

                normalized[motor_i] = 2 * (angles[motor_i] - low) / (high - low) - 1.0

        return np.clip(normalized, -1.0, 1.0)   # clipped to prevent bugs with wrong input value or floating point calculations

    def _apply_action(self, action):
        for i in range(6):
            for joint_index, joint_type in enumerate(self.joint_types):
                motor_i = i * 3 + joint_index
                delta = action[motor_i] * self._action_scales[motor_i]
                target = BASE_POSE[joint_index] + delta

                low, high = JOINT_LIMITS[joint_type]
                target = float(np.clip(target, low, high))

                if joint_type == "coax":
                    target *= LEG_SIDES[i]

                self.motors[motor_i].setPosition(target)

    def _compute_reward(self, obs, action):
        vel_x = obs[6]
        roll, pitch = obs[3], obs[4]

        r_forward = vel_x * 5           # 5 and other multiplied values are weights
        r_stable = -abs(roll) * 0.5 - abs(pitch) * 0.5
        r_smooth = -np.sum(np.abs(action - self._prev_action)) * 0.05
        r_energy = -np.sum(action ** 2) * 0.01

        reward = r_forward + r_stable + r_smooth + r_energy
        return reward
    
    def _is_terminated(self, obs):
        roll, pitch = obs[3], obs[4]
        height = obs[2]

        flalen = (abs(roll) > FALL_THRESHOLD_RAD or
                  abs(pitch) > FALL_THRESHOLD_RAD or
                  height < MIN_HEIGHT)

        return flalen

    def reset(self, seed=None, option=None):
        super().reset(seed=seed)

        robot_node = self.robot.getSelf()
        trans_field = robot_node.getField("translation")
        rot_field = robot_node.getField("rotation")

        self.robot.simulationResetPhysics()
        robot_node.resetPhysics()

        trans_field.setSFVec3f([0, 0, 0.15])
        rot_field.setSFRotation([0, 0, 1, 0])

        base_action = np.zeros(18, dtype=np.float32)
        self._apply_action(base_action)

        for i in range(10):             # few steps to stabilize simulation
            self.robot.step(TIME_STEP)

        self._step_count = 0
        self._prev_action = np.zeros(18, dtype=np.float32)
        self._prev_position = np.array(self.gps.getValues(), dtype=np.float32)

        observation = self._get_observation()

        info = {}

        return observation, info

    def step(self, action):
        action = np.clip(action, -1.0, 1.0).astype(np.float32)
        self._apply_action(action)

        self.robot.step(TIME_STEP)
        self._step_count += 1

        observation = self._get_observation()
        reward = self._compute_reward(observation, action)
        terminated = self._is_terminated(observation)
        truncated = self._step_count >= MAX_EPISODE_STEPS

        self._prev_action = action.copy()

        info = {
            "vel_x":    float(observation[6]),
            "roll":     float(observation[3]),
            "pitch":    float(observation[4]),
            "step":     self._step_count,
        }

        return observation, reward, terminated, truncated, info

    def close(self):
        pass

# try:

#     env = HexapodEnv()

    # robot = Supervisor()
    # timestep = int(TIME_STEP)
    # print(f"dziala {int(robot.getBasicTimeStep())}")

    # if robot.step(timestep) != -1:
    #     print("symulaja chodzi")

    # gps = robot.getDevice("gps")
    # gps.enable(timestep)

    # imu = robot.getDevice("inertial unit")
    # imu.enable(timestep)

    # gyro = robot.getDevice("gyro")
    # gyro.enable(timestep)

    # motor = robot.getDevice("femur_lf_motor")
    # motor.setPosition(0.5)

    # while robot.step(timestep) != -1:
    #     values_gps = gps.getValues()
    #     values_gyro = gyro.getValues()
    #     values_imu = imu.getRollPitchYaw()

    #     gps_fmt  = [f"{v:8.3f}" for v in values_gps]
    #     imu_fmt  = [f"{v:8.3f}" for v in values_imu]
    #     gyro_fmt = [f"{v:8.3f}" for v in values_gyro]

    #     print(f"GPS: {gps_fmt}  IMU: {imu_fmt}  Gyro: {gyro_fmt}")

# except Exception as e: 
#     print(f"Błąd {e}")