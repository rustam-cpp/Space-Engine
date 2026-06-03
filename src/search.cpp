#include "search.h"
#include "constants.h"
#include "converts.h"
#include "move.h"
#include "movegen.h"
#include "evaluation.h"
#include "types.h"
#include <algorithm>
#include <chrono>

bool is_running = false;
bool stop = false;

long st, et;
long timeToThink;

Move killer[MAX_PLY][2];
int history[2][64][64];

int64_t perft(Depth init, Depth depth, Position pos, rt* RT) {
  // leaf
  if (depth == 0) return 1;
  // all legal moves
  std::vector<Move> moves;
  generateMoves<false, false>(pos, moves, RT);
  int64_t ans = 0;
  // we iterate over all moves
  for (const Move& move : moves) {
    // play move
    doMove(pos, move, RT);
    // do it recursively
    int64_t cur = perft(init, depth - 1, pos, RT);
    ans += cur;
    if (depth == init) {
      std::cout << convertMoveToString(move);
      std::cout << ": " << cur << std::endl;
    }
    // undo move
    undoMove(pos, move, RT);
  }
  if (depth == init) {
    std::cout << "nodes: " << ans << std::endl;
  }
  return ans;
}

void pickMove(
  bool WhiteToMove,
  std::vector<Move>& moves,
  int start,
  const Move& ttMove,
  Depth ply
) {

  int bestIndex = start;
  Eval best = -INF;

  for (int i = start; i < (int)moves.size(); i++) {

    Move m = moves[i];

    Eval score = simp[getType(m.PromotedTo)];

    // tt move
    if (m == ttMove) {
      bestIndex = i;
      break;
    // MVV-LVA score
    } else if (getType(m.Captured) != NONE) {
      score += capture + mvv_lva[getType(m.Captured)][getType(m.Moved)];
    // killer moves
    } else if (ply >= 0) {
      if (m == killer[ply][0]) {
        score = killer1;
      } else if (m == killer[ply][1]) {
        score = killer2;
      } else {
        score = history[WhiteToMove][m.From][m.To];
      }
    }

    if (score > best) {
      best = score;
      bestIndex = i;
    }

  }

  std::swap(moves[start], moves[bestIndex]);

}

Depth seldepth;

Eval qsearch(
  Position pos,
  Eval alpha, Eval beta,
  int64_t& nodes,
  tt* TT, rt* RT,
  Depth ply
) {

  if (!is_running) return 0;

  nodes++;

  if ((nodes & 2047) == 0) {
    et = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    if ((et - st) / 1'000'000 >= timeToThink) {
      is_running = false;
      stop = true;
      return 0;
    }
  }

  seldepth = std::max(seldepth, ply);

  // if there is a draw on the board, then you need to return 0
  if (pos.isRepetitionDraw(RT) || pos.isFiftyMoveDraw())
    return 0;

  // static evaluation of position
  Eval eval = evaluation(pos);

  // fail-high
  if (eval >= beta)
    return eval;

  Eval original_alpha = alpha;

  alpha = std::max(alpha, eval);

  TTentry* entry = TT->probe(pos.ZobristHash);
  if (entry != nullptr) {
    Eval e = entry->eval;

    // mate score normalization
    if (e > Mate - 1000)
      e -= ply;
    if (e < -Mate + 1000)
      e += ply;

    if (entry->flag == EXACT)
      return e;
    if (entry->flag == LOWER && e >= beta)
      return e;
    if (entry->flag == UPPER && e <= alpha)
      return e;
  }

  std::vector<Move> moves;
  generateMoves<true, false>(pos, moves, RT);

  if (moves.empty()) {
    generateMoves<false, true>(pos, moves, RT);
    // the end
    if (moves.empty()) {
      if (inCheck(pos))
        // checkmate on the board
        return -Mate + ply;
      else
        // stalemate on the board
        return 0;
    }
    // i think .pop_back() will work
    // faster than .clear() when we
    // want to delete only 1 element
    moves.pop_back();
  }

  Move bestMove;

  for (int i = 0; i < (int)moves.size(); i++) {

    if (!is_running) break;

    pickMove(pos.WhiteToMove, moves, i, Move(), -1);

    doMove(pos, moves[i], RT);

    eval = -qsearch(
      pos,
      -beta, -alpha,
      nodes,
      TT, RT,
      ply + 1
    );

    undoMove(pos, moves[i], RT);
    
    if (eval > alpha) {
      bestMove = moves[i];
      alpha = eval;
    }

    if (eval >= beta) {
      break;
    }

  }

  Bound flag;

  if (alpha <= original_alpha)
    flag = UPPER;
  else if (alpha >= beta)
    flag = LOWER;
  else
    flag = EXACT;

  // store evaluation
  Eval sE = alpha;
  
  // mate normalization
  if (sE > Mate - 1000)
    sE += ply;

  if (sE < -Mate + 1000)
    sE -= ply;

  TT->store(pos.ZobristHash, 0, sE, flag, bestMove);

  return alpha;

}

Eval search(
  Depth depth,
  Position pos,
  Eval alpha, Eval beta,
  int64_t& nodes,
  tt* TT, rt* RT,
  Depth ply,
  Depth ext,
  std::deque<Move>& pv
) {

  if (!is_running) return 0;

  seldepth = std::max(seldepth, ply);

  nodes++;

  if ((nodes & 2047) == 0) {
    et = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    if ((et - st) / 1'000'000 >= timeToThink) {
      is_running = false;
      stop = true;
      return 0;
    }
  }

  // if there is a draw on the board, then you need to return 0
  if (pos.isRepetitionDraw(RT) || pos.isFiftyMoveDraw())
    return 0;

  // if we can use TT
  TTentry* entry = TT->probe(pos.ZobristHash);
  if (entry != nullptr && entry->depth >= depth) {
    Eval eval = entry->eval;

    // mate score normalization
    if (eval > Mate - 1000)
      eval -= ply;
    if (eval < -Mate + 1000)
      eval += ply;

    if (entry->flag == EXACT)
      return eval;
    if (entry->flag == LOWER && eval >= beta)
      return eval;
    if (entry->flag == UPPER && eval <= alpha)
      return eval;
  }
  
  // qsearch
  if (depth <= 0) {
    return qsearch(
      pos,
      alpha, beta,
      nodes,
      TT, RT,
      ply
    );
  }

  bool isCheck = inCheck(pos);

  // all legal moves
  std::vector<Move> moves;
  generateMoves<false, false>(pos, moves, RT);

  if (moves.empty()) {
    if (isCheck)
      // checkmate on the board
      return -Mate + ply;
    else
      // stalemate on the board
      return 0;
  }

  Move ttMove = Move();
  if (entry != nullptr) {
    ttMove = entry->bestMove;
  }

  // original alpha
  Eval original_alpha = alpha;

  // the future best evaluation and best move
  Eval Best = -INF;

  Move bestMove = Move();

  std::deque<Move> localPV;

  // null move pruning
  if (depth >= 3 &&
      !isCheck &&
      alpha + 1 == beta &&
      evalNonPawnMaterial(pos) != 0) {

    // en passant square
    Square eps = pos.EnPassantSquare;

    Depth r = 2 + depth / 6;

    doNullMove(pos, RT);

    Eval eval = -search(
      depth - 1 - r,
      pos,
      -beta, -beta + 1,
      nodes,
      TT, RT,
      ply + 1,
      ext,
      localPV
    );

    undoNullMove(pos, RT);

    pos.EnPassantSquare = eps;

    if (eval >= beta) {
      Best = eval;
      moves.clear();
    }

  }

  for (int i = 0; i < (int)moves.size(); i++) {

    if (!is_running) break;

    localPV.clear();

    pickMove(pos.WhiteToMove, moves, i, ttMove, ply);

    const Move move = moves[i];

    // play move
    doMove(pos, move, RT);

    // we run the recursively search

    Eval eval;

    isCheck = inCheck(pos);

    Depth e = 0;
    e += (isCheck && ext < 16 ? 1 : 0);

    if (depth >= 3 && i >= 4 && getType(move.Captured) == NONE && !isCheck) {

      // LMR (Late Move Reductions)
      Depth r = (_log2(depth) * _log2(i + 1)) / 2;
      r = std::min((int)r, 2);

      eval = -search(
        depth - 1 - r + e,
        pos,
        -alpha - 1, -alpha,
        nodes,
        TT, RT,
        ply + 1,
        ext + e,
        localPV
      );

      if (eval > alpha) {
        eval = -search(
          depth - 1 + e,
          pos,
          -beta, -alpha,
          nodes,
          TT, RT,
          ply + 1,
          ext + e,
          localPV
        );
      }

    } else {

      // PVS (Principal Variation Search)
      if (i == 0) {
        eval = -search(
          depth - 1 + e,
          pos,
          -beta, -alpha,
          nodes,
          TT, RT,
          ply + 1,
          ext + e,
          localPV
        );
      } else {

        eval = -search(
          depth - 1 + e,
          pos,
          -alpha - 1, -alpha,
          nodes,
          TT, RT,
          ply + 1,
          ext + e,
          localPV
        );

        if (eval > alpha) {
          eval = -search(
            depth - 1 + e,
            pos,
            -beta, -alpha,
            nodes,
            TT, RT,
            ply + 1,
            ext + e,
            localPV
          );
        }

      }

    }

    // undo move
    undoMove(pos, move, RT);

    // okay, maybe we have a new best move

    if (Best < eval) {
      Best = eval;
      bestMove = move;
      localPV.push_front(move);
      pv = localPV;
    }

    alpha = std::max(alpha, eval);

    // beta cutoff
    if (eval >= beta) {
      // store killer if the move is quiet
      if (!move.EnPassant &&
          getType(move.PromotedTo) == NONE &&
          getType(move.Captured) == NONE) {
        // update history
        history[pos.WhiteToMove][move.From][move.To] += depth * depth;
        // update killers
        if (!(killer[ply][0] == move)) {
          killer[ply][1] = killer[ply][0];
          killer[ply][0] = move;
        }
      }

      break;
    }
  }

  Bound flag;

  if (Best <= original_alpha)
    flag = UPPER;
  else if (Best >= beta)
    flag = LOWER;
  else
    flag = EXACT;

  // store evaluation
  Eval sE = Best;
  
  // mate normalization
  if (sE > Mate - 1000)
    sE += ply;

  if (sE < -Mate + 1000)
    sE -= ply;

  TT->store(pos.ZobristHash, depth, sE, flag, bestMove);

  return Best;

}

std::deque<Move> PV;

std::pair<Move, Eval> search_root(
  Position pos,
  Depth depth,
  Eval alpha, Eval beta,
  int64_t& nodes,
  tt* TT, rt* RT
) {

  // the search is running now
  is_running = true;

  // original alpha
  Eval oA = alpha;

  // we will look at every legal move in the position
  std::vector<Move> moves;
  generateMoves<false, false>(pos, moves, RT);

  // if it is already mate or stalemate
  if (moves.empty()) {
    return {Move(), (inCheck(pos) ? -Mate : 0)};
  }

  // if we can use TT
  TTentry* entry = TT->probe(pos.ZobristHash);

  Move ttMove = Move();
  if (entry != nullptr) {
    ttMove = entry->bestMove;
  }
  
  // and this is the best move evaluation
  Eval BestEval = -INF;
  // best move
  Move BestMove;

  for (int i = 0; i < (int)moves.size(); i++) {

    if (!is_running) return {BestMove, BestEval};

    pickMove(pos.WhiteToMove, moves, i, ttMove, -1);

    // don't forget to move
    doMove(pos, moves[i], RT);

    std::deque<Move> pv;

    // we evaluate current position
    Eval eval = -search(
      depth - 1,
      pos,
      -beta, -alpha,
      nodes,
      TT, RT,
      0, 0,
      pv
    );

    // don't forget to undo this move
    undoMove(pos, moves[i], RT);

    // if we better than all previous moves
    if (eval > BestEval) {
      BestMove = moves[i];
      BestEval = eval;
      PV = pv;
      PV.push_front(moves[i]);
    }

    alpha = std::max(alpha, eval);

    if (alpha >= beta)
      break;

    // std::cerr << convertMoveToString(move) << ": " << eval << std::endl;

  }

  Bound flag;

  if (BestEval <= oA)
    flag = UPPER;
  else if (BestEval >= beta)
    flag = LOWER;
  else
    flag = EXACT;

  TT->store(pos.ZobristHash, depth, BestEval, flag, BestMove);

  // now we are not running
  is_running = false;

  // and return values
  return {BestMove, BestEval};

}

std::pair<Move, int64_t> iterative_depening(
  Position pos,
  tt* TT, rt* RT,
  Depth maxDepth,
  long soft, long hard
) {

  memset(history, 0, sizeof(history));
  for (Depth ply = 0; ply < MAX_PLY; ply++) {
    killer[ply][0] = Move();
    killer[ply][1] = Move();
  }

  // start time
  st = std::chrono::high_resolution_clock::now().time_since_epoch().count();
  et = st;

  timeToThink = hard;

  Depth depth = 1;

  // we will return this
  std::vector<Move> temp;
  generateMoves<false, true>(pos, temp, RT);

  if (temp.empty()) {
    return {Move(), 0};
  }

  Move BestMove = temp[0];

  Eval eval = -INF;

  int64_t nodes = 0;

  stop = false;

  auto runSearch = [&](Eval alpha, Eval beta) {

    seldepth = 0;

    std::pair<Move, Eval> result = {Move(), -INF};

    if (!(is_running || stop)) {
      result = search_root(pos, depth, alpha, beta, nodes, TT, RT);
    }

    if (is_running || stop) {
      is_running = false;
    }

    return result;

  };

  do {

    // aspiration window

    std::pair<Move, Eval> result;

    if (depth <= 4) {

      result = runSearch(-INF, INF);

    } else {

      Eval score = eval;
      Eval window = 20;

      Eval alpha = score - window;
      Eval beta = score + window;

      bool needFullSearch = true;

      while (window <= 80) {

        result = runSearch(alpha, beta);

        if (is_running || stop) {
          is_running = false;
          needFullSearch = false;
          break;
        }

        score = result.second;

        if (score <= alpha) {
          alpha -= window;
        } else if (score >= beta) {
          beta += window;
        } else {
          needFullSearch = false;
          break;
        }

        window *= 2;

      }

      if (needFullSearch) {
        result = runSearch(-INF, INF);
      }

    }

    // time is up  or stop command received
    if (is_running || stop) {
      is_running = false;
      break;
    }

    BestMove = result.first;
    eval = result.second;

    et = std::chrono::high_resolution_clock::now().time_since_epoch().count();

    long Time = (et - st) / 1'000'000;
    int64_t nps = (Time > 0) ? (nodes * 1000 / Time) : 0;

    // UCI

    std::cout << "info  depth " << toLen(depth, 4)
              << " seldepth " << toLen(seldepth, 4)
              << " nodes " << toLen(nodes, 11)
              << " nps " << toLen(nps, 9)
              << " hashfull " << toLen(TT->hashfull(), 4)
              << " time " << toLen(Time, 8)
              << " score " << score(eval)
              << " pv ";
    for (const Move& move : PV) {
      std::cout << convertMoveToString(move) << ' ';
    }
    std::cout << std::endl;

    depth++;

    // if we potentially will time out
    if (Time > soft) break;

  } while (depth <= maxDepth);

  return {BestMove, nodes};

}

void stopCommand() {
  stop = true;
  is_running = false;
}
