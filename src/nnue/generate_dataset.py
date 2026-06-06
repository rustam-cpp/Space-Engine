import chess
import chess.engine
import multiprocessing as mp
from tqdm import tqdm

STOCKFISH_PATH = "stockfish"
INPUT = "dataset/fen.txt"
OUTPUT = "dataset/dataset.txt"

TIME_LIMIT = 0.03
THREADS_PER_ENGINE = 1
BATCH_SIZE = 500


def chunk_list(lst, chunk_size):
  for i in range(0, len(lst), chunk_size):
    yield lst[i:i + chunk_size]


def eval_batch(fens):
  engine = chess.engine.SimpleEngine.popen_uci(STOCKFISH_PATH)
  engine.configure({"Threads": THREADS_PER_ENGINE})

  results = []

  for fen in fens:
    try:
      board = chess.Board(fen)

      info = engine.analyse(
        board,
        chess.engine.Limit(time=TIME_LIMIT)
      )

      score = info["score"].white().score(mate_score=30000)
      score = score if score is not None else 0

      results.append(f"{fen}|{score}")

    except Exception:
      continue

  engine.quit()
  return results


def main():
  with open(INPUT) as f:
    fens = [line.strip() for line in f if line.strip()]

  batches = list(chunk_list(fens, BATCH_SIZE))

  workers = 8

  print(f"FENs: {len(fens)}")
  print(f"Batches: {len(batches)}")
  print(f"Workers: {workers}")

  with mp.Pool(workers) as pool:

    all_results = list(
      tqdm(
        pool.imap(eval_batch, batches),
        total=len(batches),
        desc="Stockfish labeling"
      )
    )

  with open(OUTPUT, "w") as out:
    for batch in all_results:
      for line in batch:
        out.write(line + "\n")

  print("Done!")


if __name__ == "__main__":
  main()