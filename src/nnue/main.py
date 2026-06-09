import torch
import torch.nn as nn
from tqdm import tqdm
import time

INPUT = 773
HIDDEN = 128


class NNUE(nn.Module):
  def __init__(self):
    super().__init__()
    self.fc1 = nn.Linear(INPUT, HIDDEN)
    self.fc2 = nn.Linear(HIDDEN, 1)

  def forward(self, x):
    return self.fc2(torch.relu(self.fc1(x)))


def load_data():
  print("Loading dataset...")
  X = torch.load("dataset/x.bin")
  Y = torch.load("dataset/y.bin")
  print(f"Loaded: {X.shape[0]} samples")
  return X, Y


def train():
  model = NNUE()

  optimizer = torch.optim.Adam(
    model.parameters(),
    lr=3e-4,
    weight_decay=1e-5
  )

  loss_fn = nn.SmoothL1Loss()

  X, Y = load_data()

  n = X.size(0)
  split = int(n * 0.9)

  X_train, Y_train = X[:split], Y[:split]
  X_val, Y_val = X[split:], Y[split:]

  batch_size = 16384
  epochs = 50

  for epoch in range(epochs):
    model.train()

    idx = torch.randperm(X_train.size(0))

    total_loss = 0
    start_time = time.time()

    pbar = tqdm(
      range(0, X_train.size(0), batch_size),
      desc=f"Epoch {epoch}",
      unit="batch"
    )

    for i in pbar:
      batch_idx = idx[i:i+batch_size]

      x_batch = X_train[batch_idx]
      y_batch = Y_train[batch_idx]

      pred = model(x_batch)
      loss = loss_fn(pred, y_batch)

      optimizer.zero_grad()
      loss.backward()
      optimizer.step()

      total_loss += loss.item() * x_batch.size(0)

      elapsed = time.time() - start_time
      seen = i + batch_size
      speed = seen / elapsed if elapsed > 0 else 0

      pbar.set_postfix(
        loss=f"{loss.item():.4f}",
        avg_loss=f"{total_loss / seen:.4f}",
        speed=f"{speed:.0f} samp/s"
      )

    total_loss /= X_train.size(0)

    model.eval()
    with torch.no_grad():
      val_pred = model(X_val)
      val_loss = loss_fn(val_pred, Y_val).item()

    print(f"\nEpoch {epoch} done")
    print(f"train_loss={total_loss:.4f}")
    print(f"val_loss={val_loss:.4f}")

    torch.save(model.state_dict(), "nnue.pt")
    print("saved nnue.pt\n")


if __name__ == "__main__":
  train()