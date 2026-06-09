import torch
import torch.nn as nn
import chess
from tqdm import tqdm
import random

INPUT = 773
HIDDEN = 128


class NNUE(nn.Module):
  def __init__(self):
    super().__init__()
    self.fc1 = nn.Linear(INPUT, HIDDEN)
    self.fc2 = nn.Linear(HIDDEN, 1)

  def forward(self, x):
    x = torch.relu(self.fc1(x))
    return self.fc2(x)


cache = {}


def fen_to_tensor(fen):
  if fen in cache:
    return cache[fen]

  board = chess.Board(fen)
  x = torch.zeros(INPUT)

  for sq in chess.SQUARES:
    piece = board.piece_at(sq)
    if not piece:
      continue

    piece_type = piece.piece_type - 1

    color_offset = 0
    if piece.color == chess.WHITE:
      color_offset = 6

    idx = (color_offset + piece_type) * 64 + sq
    x[idx] = 1

  if board.turn == chess.WHITE:
    x[768] = 1.0

  if board.has_kingside_castling_rights(chess.BLACK):
    x[769] = 1.0

  if board.has_queenside_castling_rights(chess.BLACK):
    x[770] = 1.0

  if board.has_kingside_castling_rights(chess.WHITE):
    x[771] = 1.0

  if board.has_queenside_castling_rights(chess.WHITE):
    x[772] = 1.0

  cache[fen] = x

  return x


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

  return data


def evaluate(model, data, loss_fn):
  model.eval()
  total_loss = 0.0

  with torch.no_grad():
    for fen, score in data:
      x = fen_to_tensor(fen)
      y = torch.tensor(score / 1000.0)

      pred = model(x)
      loss = loss_fn(pred, y.unsqueeze(0))
      total_loss += loss.item()

  return total_loss / len(data)


def train():
  model = NNUE()
  optimizer = torch.optim.Adam(model.parameters(),
                 lr=3e-4,
                 weight_decay=1e-5)

  loss_fn = nn.SmoothL1Loss()

  data = load_data("dataset/dataset.txt")
  random.shuffle(data)

  split = int(len(data) * 0.9)
  train_data = data[:split]
  val_data = data[split:]

  batch_size = 256
  epochs = 50

  for epoch in range(epochs):

    total_loss = 0
    model.train()

    random.shuffle(train_data)

    loop = tqdm(range(0, len(train_data), batch_size), desc=f"Epoch {epoch}")

    for i in loop:
      batch = train_data[i:i + batch_size]

      x_batch = []
      y_batch = []

      for fen, score in batch:
        x_batch.append(fen_to_tensor(fen))
        y_batch.append(torch.tensor(score / 1000.0))

      x_batch = torch.stack(x_batch)
      y_batch = torch.stack(y_batch).unsqueeze(1)

      pred = model(x_batch)
      loss = loss_fn(pred, y_batch)

      optimizer.zero_grad()
      loss.backward()
      optimizer.step()

      total_loss += loss.item() * x_batch.size(0)
      loop.set_postfix(loss=loss.item())

    total_loss /= len(train_data)

    val_loss = evaluate(model, val_data, loss_fn)

    print(f"\nepoch {epoch} done")
    print(f"train_loss={total_loss:.4f}")
    print(f"val_loss={val_loss:.4f}")

    torch.save(model.state_dict(), "nnue.pt")
    print("model saved -> nnue.pt\n")


if __name__ == "__main__":
  train()