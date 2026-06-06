import chess.pgn
import chess
import zstandard as zstd
import io
import random

INPUT = "dataset/lichess_db_standard_rated_2013-01.pgn.zst"
OUTPUT = "dataset/dataset.txt"

with open(INPUT, "rb") as f:
  dctx = zstd.ZstdDecompressor()
  stream = dctx.stream_reader(f)

  text_stream = io.TextIOWrapper(stream, encoding="utf-8")

  with open(OUTPUT, "w") as out:

    while True:
      game = chess.pgn.read_game(text_stream)
      if game is None:
        break

      board = game.board()
      movecount = random.randint(0, 39)

      for move in game.mainline_moves():
        board.push(move)
        movecount += 1

        if movecount % 40 == 0:
          out.write(board.fen() + "\n")