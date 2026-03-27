import os
import argparse
import numpy as np
from datetime import datetime

import sys 
from pathlib import Path

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

PPO_CONFIG = dict(
    n_steps = 128,       # 2048      # steps before actualization
    batch_size = 64,            # size of mini batch before actialization
    n_epochs = 10,              
    gamma = 0.99,
    gae_lambda = 0.5,
    clip_range = 0.2,
    ent_coef = 0.01,
    learning_rate = 3e-4,
    policy_kwargs = dict(
        net_arch = dict(
            pi = [256, 256],
            vf = [256, 256],
        )
    ),
)

TOTAL_TIMESTEPS = 20000
CHECKPOINT_FREQ = 5000
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
        model = PPO("MlpPolicy", train_env, **PPO_CONFIG)

    run_name = datetime.now().strftime("%Y%m%d_%H%M%S")

    checkpoint_cb = CheckpointCallback(
        save_freq=CHECKPOINT_FREQ,
        save_path=f"checkpoints/{run_name}/",
        name_prefix="hexapod_ppo",
        save_vecnormalize=True,
        verbose=1,
    )

    callbacks = CallbackList([checkpoint_cb])

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

    model = PPO.load(model_path, env=env)

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