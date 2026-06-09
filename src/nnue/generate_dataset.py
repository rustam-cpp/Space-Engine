import chess
import chess.engine
import multiprocessing as mp
from tqdm import tqdm

STOCKFISH_PATH = "stockfish"

INPUT = "dataset/fen.txt"
OUTPUT = "dataset/dataset.txt"

BATCH_SIZE = 800
WORKERS = 8

NODES_LIMIT = 800

engine = None


def init_engine():
  global engine
  engine = chess.engine.SimpleEngine.popen_uci(STOCKFISH_PATH)

  engine.configure({
    "Threads": 1,
    "Hash": 64
  })


def eval_batch(fens):
  results = []

  local_engine = engine

  for fen in fens:
    try:
      board = chess.Board(fen)

      info = local_engine.analyse(
        board,
        chess.engine.Limit(nodes=NODES_LIMIT)
      )

      score_obj = info["score"].relative
      score = score_obj.score(mate_score=30000)

      if score is None:
        score = 0

      results.append(f"{fen}|{score}")

    except Exception:
      continue

  return results


def chunk_list(lst, chunk_size):
  for i in range(0, len(lst), chunk_size):
    yield lst[i:i + chunk_size]


def main():
  with open(INPUT) as f:
    fens = [line.strip() for line in f if line.strip()]

  batches = list(chunk_list(fens, BATCH_SIZE))

  print(f"FENs: {len(fens)}")
  print(f"Batches: {len(batches)}")
  print(f"Workers: {WORKERS}")
  print(f"Batch size: {BATCH_SIZE}")

  with mp.Pool(
    processes=WORKERS,
    initializer=init_engine,
    maxtasksperchild=200
  ) as pool:

    with open(OUTPUT, "w") as out:
      for batch_result in tqdm(
        pool.imap_unordered(eval_batch, batches, chunksize=1),
        total=len(batches),
        desc="Stockfish labeling"
      ):
        for line in batch_result:
          out.write(line + "\n")

  print("Done!")


if __name__ == "__main__":
  mp.set_start_method("fork", force=True)
  main()