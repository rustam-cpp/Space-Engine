import torch
import chess
from tqdm import tqdm
import time

INPUT = 772


def fen_to_tensor(fen):
  board = chess.Board(fen)
  x = torch.zeros(INPUT)

  pieces = board.piece_map()

  for sq, piece in pieces.items():

    color_offset = 0
    if piece.color == chess.WHITE:
      color_offset = 6

    idx = (piece.piece_type - 1 + color_offset) * 64 + sq

    x[idx] = 1

  x[INPUT - 4] = board.has_kingside_castling_rights(chess.BLACK)
  x[INPUT - 3] = board.has_queenside_castling_rights(chess.BLACK)
  x[INPUT - 2] = board.has_kingside_castling_rights(chess.WHITE)
  x[INPUT - 1] = board.has_queenside_castling_rights(chess.WHITE)

  return x


def main():
  with open("dataset/dataset.txt") as f:
    lines = f.readlines()

  X = []
  Y = []

  print(f"Total samples: {len(lines)}")

  start = time.time()

  pbar = tqdm(lines, desc="Precomputing dataset", unit="fen")

  for i, line in enumerate(pbar):
    fen, score = line.strip().split("|")

    X.append(fen_to_tensor(fen))
    Y.append(float(score) / 1000.0)

    if i % 1000 == 0 and i > 0:
      speed = i / (time.time() - start)
      pbar.set_postfix(speed=f"{speed:.1f} fen/s")

  X = torch.stack(X)
  Y = torch.tensor(Y).unsqueeze(1)

  print("Saving tensors...")

  torch.save(X, "dataset/x.bin")
  torch.save(Y, "dataset/y.bin")

  print("Saved -> dataset/x.bin, dataset/y.bin")


if __name__ == "__main__":
  main()