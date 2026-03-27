import os
import sys

from hexapod_env import HexapodEnv

def run_random():
    print("[hexapod_rl] Random actions mode")

    env = HexapodEnv()
    obs, _ = env.reset()

    for step in range(200):
        action = env.action_space.sample()
        observation, reward, terminated, truncated, info = env.step(action)

        if(step % 20 == 0):
            print(f"step: {step:3d} | reward: {reward:6.3f} | vel_x: {info['vel_x']:.3f}")

        if terminated or truncated:
            print(f"[hexapod_rl] Episode finished after {step} steps")
            observation, _ = env.reset() 

    env.close()
    print("[hexapod_rl] Random actions test finished")



if __name__ == "__main__":
    run_random()