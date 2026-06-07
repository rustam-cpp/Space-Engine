import torch
import torch.nn as nn
import chess
from tqdm import tqdm

INPUT = 768
HIDDEN = 128

class NNUE(nn.Module):
  def __init__(self):
    super().__init__()
    self.fc1 = nn.Linear(INPUT, HIDDEN)
    self.fc2 = nn.Linear(HIDDEN, 1)

  def forward(self, x):
    x = torch.relu(self.fc1(x))
    return self.fc2(x)


def fen_to_tensor(fen):
  board = chess.Board(fen)
  x = torch.zeros(INPUT)

  for sq in chess.SQUARES:
    piece = board.piece_at(sq)
    if not piece:
      continue

    piece_type = piece.piece_type - 1  # 0..5

    color_offset = 0
    if piece.color == chess.WHITE:
      color_offset = 6  # white pieces start after black

    idx = (color_offset + piece_type) * 64 + sq

    x[idx] = 1

  return x


mean=0

def load_data(path):
  data = []
  with open(path) as f:
    for line in f:
      fen, score = line.strip().split("|")
      data.append((fen, float(score)))

  scores = [s for _, s in data]
  print("MIN:", min(scores))
  print("MAX:", max(scores))
  print("MEAN:", sum(scores) / len(scores))
  mean = sum(scores) / len(scores)

  return data


def train():
  model = NNUE()
  optimizer = torch.optim.Adam(model.parameters(), lr=1e-3)
  loss_fn = nn.MSELoss()

  data = load_data("dataset/dataset.txt")

  batch_size = 256
  epochs = 50

  for epoch in range(epochs):

    total_loss = 0

    loop = tqdm(range(0, len(data), batch_size), desc=f"Epoch {epoch}")

    for i in loop:
      batch = data[i:i+batch_size]

      x_batch = []
      y_batch = []

      for fen, score in batch:
        x_batch.append(fen_to_tensor(fen))
        y_batch.append(min(30, max(-30, score / 100)))

      x_batch = torch.stack(x_batch)
      y_batch = torch.tensor(y_batch).unsqueeze(1)

      pred = model(x_batch)
      loss = loss_fn(pred, y_batch)

      optimizer.zero_grad()
      loss.backward()
      optimizer.step()

      total_loss += loss.item()

      loop.set_postfix(loss=loss.item())

    print(f"epoch {epoch} done, loss={total_loss:.4f}")

    file = "nnue.pt"

    torch.save(model.state_dict(), file)
    print("model saved -> " + file)


if __name__ == "__main__":
  train()