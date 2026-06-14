import os
import argparse
import numpy as np
from datetime import datetime

import sys 
from pathlib import Path

from stable_baselines3.common.callbacks import BaseCallback

class HexapodMetricsCallback(BaseCallback):
    def __init__(self, verbose=0):
        super().__init__(verbose)
        self._vel_x = []
        self._roll = []
        self._pitch = []
        self._heights = []
        self._rewards = []

    def _on_step(self) -> bool:
        infos = self.locals.get("infos", [])
        rewards = self.locals.get("rewards", [])

        if rewards is not None:
            self._rewards.extend(rewards.tolist())

        for info in infos:
            if "vel_x" in info:
                self._vel_x.append(info["vel_x"])
                self._roll.append(abs(info["roll"]))
                self._pitch.append(abs(info["pitch"]))
            if "height" in info:
                self._heights.append(info["height"])

        if self.num_timesteps % 512 == 0 and self._rewards:
            self.logger.record("hexapod/reward_mean", np.mean(self._rewards))
            self.logger.record("hexapod/reward_min",  np.min(self._rewards))
            self.logger.record("hexapod/reward_max",  np.max(self._rewards))

            if self._vel_x:
                self.logger.record("hexapod/vel_y",   np.mean(self._vel_x))
                self.logger.record("hexapod/roll",     np.mean(self._roll))
                self.logger.record("hexapod/pitch",    np.mean(self._pitch))
            if self._heights:
                self.logger.record("hexapod/height",   np.mean(self._heights))

            self.logger.dump(self.num_timesteps)

            self._rewards.clear()
            self._vel_x.clear()
            self._roll.clear()
            self._pitch.clear()
            self._heights.clear()

        return True

root_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(root_dir))

from stable_baselines3 import PPO
from stable_baselines3.common.monitor import Monitor
from stable_baselines3.common.vec_env import DummyVecEnv, VecNormalize
from stable_baselines3.common.callbacks import (
    CheckpointCallback,
    EvalCallback,
    CallbackList,
)

from webotsSimulation.controllers.hexapod_rl.hexapod_env import HexapodEnv

def linear_schedule(initial_value):
    def schedule(progress):          # progress: 1.0 → 0.0
        return initial_value * progress
    return schedule


PPO_CONFIG = dict(
    n_steps = 512,       # 2048      # steps before actualization
    batch_size = 64,            # size of mini batch before actialization
    n_epochs = 10,              
    gamma = 0.99,
    gae_lambda = 0.95,
    clip_range = 0.2,
    ent_coef = 0.01,
    learning_rate = linear_schedule(3e-4),
    policy_kwargs = dict(
        net_arch = dict(
            pi = [128, 128],
            vf = [128, 128],
        )
    ),
)

TOTAL_TIMESTEPS = 2000000
CHECKPOINT_FREQ = 50000
# EVAL_FREQ = 25000
# EVAL_EPISODES = 5

def make_env():
    env = HexapodEnv()
    env = Monitor(env, filename="logs/monitor")
    return env

def train(resume_path=None):
    os.makedirs("logs", exist_ok=True)
    os.makedirs("checkpoints", exist_ok=True)
    os.makedirs("models/saved", exist_ok=True)

    base_env = DummyVecEnv([make_env])

    train_env = VecNormalize(
        base_env,
        norm_obs=True,
        norm_reward=True,
        clip_obs=10.0,
        clip_reward=10.0,
    )

    if resume_path:
        print(f"[train INFO] Read model from: {resume_path}")
        model = PPO.load(resume_path, env=train_env)

        stats_path = resume_path.replace(".zip", "_vecnormalize.pkl")
        if os.path.exists(stats_path):
            train_env = VecNormalize.load(stats_path, base_env)
            print(f"[train INFO] Read statistic from: {stats_path}")
    else: 
        model = PPO("MlpPolicy", train_env, tensorboard_log="logs/tensorboard", **PPO_CONFIG)

    run_name = datetime.now().strftime("%Y%m%d_%H%M%S")

    checkpoint_cb = CheckpointCallback(
        save_freq=CHECKPOINT_FREQ,
        save_path=f"checkpoints/{run_name}/",
        name_prefix="hexapod_ppo",
        save_vecnormalize=True,
        verbose=1,
    )

    metrics_cb = HexapodMetricsCallback()

    callbacks = CallbackList([checkpoint_cb, metrics_cb])

    print("\n")
    print(f"[train INFO] Starting training PPO")
    print(f"[train INFO] Total steps: {TOTAL_TIMESTEPS:,}")
    print(f"[train INFO] Run: {run_name}")
    print("\n")

    model.learn(
        total_timesteps=TOTAL_TIMESTEPS,
        callback=callbacks,
        reset_num_timesteps=resume_path is None,
        progress_bar=True,
        tb_log_name=run_name,
    )

    final_path = f"models/saved/hexapod_final_{run_name}"
    model.save(final_path)
    train_env.save(f"{final_path}_vecnormalize.pkl")
    print(f"[train INFO] Model saved: {final_path}.zip")

    return model, train_env, final_path

def evaluate(model_path, n_episodees=10):
    env = DummyVecEnv([make_env])
    stats_path = model_path.replace(".zip", "_vecnormalize.pkl")

    if os.path.exists(stats_path):
        env = VecNormalize.load(stats_path, env)
        env.training = False
        env.norm_reward = False

    model = PPO.load(model_path, env=env, tensorboard_log="logs/tensorboard")


    rewards = []
    for ep in range(n_episodees):
        observation = env.reset()
        ep_reward = 0.0
        done = False

        while not done:
            action, _ = model.predict(observation, deterministic=True)
            observation, reward, done, info = env.step(action)
            ep_reward += reward[0]

        rewards.append(ep_reward)
        print(f"[train INFO] Episode {ep+1:2d}: reward = {ep_reward:.2f}")

    print(f"\n Average reward: {np.mean(rewards):2f} | Standard deviation: {np.std(rewards):.2f}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Hexapod PPO Training")
    parser.add_argument("--resume", type=str, default=None)
    parser.add_argument("--eval", type=str, default=None)
    args = parser.parse_args()

    if args.eval:
        evaluate(args.eval)
    else:
        train(resume_path=args.resume)