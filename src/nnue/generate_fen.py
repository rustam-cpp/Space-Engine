import os
import re
import mmap
import chess
import random
from tqdm import tqdm
from concurrent.futures import ProcessPoolExecutor, as_completed

INPUT   = "dataset/lichess_db_standard_rated_2016-03.pgn"
OUTPUT  = "dataset/fen.txt"
THREADS = 8
CHUNK_GAMES = 2000

_RE_COMMENT   = re.compile(r'\{[^}]*\}')
_RE_VARIATION = re.compile(r'\([^)]*\)')
_RE_MOVENUM   = re.compile(r'^\d+\.+$')
_RESULT_TOKENS = frozenset({'1-0', '0-1', '1/2-1/2', '*'})


def _worker(args: tuple) -> list[str]:
  filepath, start_byte, end_byte = args

  with open(filepath, 'rb') as f:
    f.seek(start_byte)
    raw = f.read(end_byte - start_byte).decode('utf-8', errors='replace')

  results: list[str] = []
  parts = raw.split('\n[Event ')
  for i, pgn_text in enumerate(parts):
    if i > 0:
      pgn_text = '[Event ' + pgn_text
    try:
      lines = pgn_text.splitlines()
      move_lines = [l for l in lines if l and l[0] != '[']
      if not move_lines:
        continue
      movetext = ' '.join(move_lines)
      movetext = _RE_COMMENT.sub('', movetext)
      movetext = _RE_VARIATION.sub('', movetext)
      tokens = [
        t for t in movetext.split()
        if t not in _RESULT_TOKENS and not _RE_MOVENUM.match(t)
      ]
      if not tokens:
        continue
      board = chess.Board()
      movecount = random.randint(0, 39)
      for san in tokens:
        board.push_san(san)
        movecount += 1
        if movecount % 40 == 0:
          results.append(board.fen())
    except Exception:
      pass
  return results


def _build_ranges(filepath: str, chunk_games: int) -> tuple[list, int]:
  fsize = os.path.getsize(filepath)
  marker = b'[Event '

  with open(filepath, 'rb') as f:
    mm = mmap.mmap(f.fileno(), 0, access=mmap.ACCESS_READ)
    offsets = [0]
    pos = 0
    while True:
      idx = mm.find(marker, pos + 1)
      if idx == -1:
        break
      offsets.append(idx)
      pos = idx
    mm.close()

  n = len(offsets)
  ranges = []
  i = 0
  while i < n:
    s = offsets[i]
    e = offsets[i + chunk_games] if i + chunk_games < n else fsize
    ranges.append((filepath, s, e))
    i += chunk_games

  return ranges, n


def main() -> None:
  print("Scanning file offsets...")
  ranges, n_games = _build_ranges(INPUT, CHUNK_GAMES)
  print(f"Found {n_games:,} games → {len(ranges)} chunks of ~{CHUNK_GAMES}, launching {THREADS} workers")

  pbar = tqdm(total=n_games, desc="Games", unit="game", smoothing=0.1)

  with open(OUTPUT, 'w', buffering=1024 * 1024) as out:
    with ProcessPoolExecutor(max_workers=THREADS) as executor:
      futures = {executor.submit(_worker, r): r for r in ranges}
      for future in as_completed(futures):
        fens = future.result()
        if fens:
          out.write('\n'.join(fens) + '\n')
        pbar.update(CHUNK_GAMES)

  pbar.close()


if __name__ == "__main__":
  main()