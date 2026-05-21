#include "search.h"
#include "constants.h"
#include "converts.h"
#include "move.h"
#include "movegen.h"
#include "evaluation.h"
#include "types.h"
#include <algorithm>
#include <chrono>
#include <future>
#include <thread>

std::atomic<bool> is_running = false;
std::atomic<bool> stop = false;

int64_t perft(Depth depth, Position pos) {
  // leaf
  if (depth == 0) return 1;
  // all legal moves
  std::vector<Move> moves;
  generateMoves<false, false>(pos, moves);
  // if depth = 1 than the answer
  // is count of all legal moves
  if (depth == 1) return moves.size();
  // answer of current node if depth > 1
  int64_t ans = 0;
  // we iterate over all moves
  for (const Move& move : moves) {
    // play move
    doMove(pos, move);
    // do it recursively
    ans += perft(depth - 1, pos);
    // undo move
    undoMove(pos, move);
  }
  return ans;
}

inline Eval getScore(const Move& move) {
               // the worst possibillity is to lose moved piece
  Eval score = - simp[getType(move.Moved)]
               // but capturing oponent's piece is very good
               + simp[getType(move.Captured)] * 10
               // and promoting is pretty good
               + simp[getType(move.PromotedTo)] * 4;
  return score;
}

void pickMove(std::vector<Move>& moves, int start, const Move& ttMove) {

  int bestIndex = start;
  Eval best = -INF;

  for (int i = start; i < (int)moves.size(); i++) {

    Move m = moves[i];
    Eval score = getScore(m);

    if (m == ttMove) {
      bestIndex = i;
      break;
    } else if (score > best) {
      best = score;
      bestIndex = i;
    }

  }

  std::swap(moves[start], moves[bestIndex]);

}

Eval qsearch(Position pos, Eval a, Eval b, int64_t& nodes, tt* TT, Depth ply) {

  if (!is_running) return 0;

  nodes++;

  // if there is a draw on the board, then you need to return 0
  if (pos.isRepetitionDraw() || pos.isFiftyMoveDraw())
    return 0;

  // static evaluation of position
  Eval eval = evaluation(pos);

  // fail-high
  if (eval >= b)
    return eval;

  Eval oA = a;

  a = std::max(a, eval);

  Entry* entry = TT->probe(pos.ZobristHash);
  if (entry != nullptr) {
    Eval e = entry->eval;

    // mate score normalization
    if (e > Mate - 1000)
      e -= ply;
    if (e < -Mate + 1000)
      e += ply;

    if (entry->flag == EXACT)
      return e;
    if (entry->flag == LOWER && e >= b)
      return e;
    if (entry->flag == UPPER && e <= a)
      return e;
  }

  std::vector<Move> moves;
  generateMoves<true, false>(pos, moves);

  if (moves.empty()) {
    generateMoves<false, true>(pos, moves);
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

    pickMove(moves, i, Move());

    doMove(pos, moves[i]);

    eval = -qsearch(pos, -b, -a, nodes, TT, ply + 1);

    undoMove(pos, moves[i]);
    
    if (eval > a) {
      bestMove = moves[i];
      a = eval;
    }

    if (eval >= b) {
      break;
    }
  }

  Bound flag;

  if (a <= oA)
    flag = UPPER;
  else if (a >= b)
    flag = LOWER;
  else
    flag = EXACT;

  // store evaluation
  Eval sE = a;
  
  // mate normalization
  if (sE > Mate - 1000)
    sE += ply;

  if (sE < -Mate + 1000)
    sE -= ply;

  TT->store(pos.ZobristHash, 0, sE, flag, bestMove);

  return a;

}

Eval search(Depth depth, Position pos, Eval a, Eval b, int64_t& nodes, tt* TT, Depth ply) {

  if (!is_running) return 0;

  nodes++;

  // if there is a draw on the board, then you need to return 0
  if (pos.isRepetitionDraw() || pos.isFiftyMoveDraw())
    return 0;

  // if we can use TT
  Entry* entry = TT->probe(pos.ZobristHash);
  if (entry != nullptr && entry->depth >= depth) {
    Eval eval = entry->eval;

    // mate score normalization
    if (eval > Mate - 1000)
      eval -= ply;
    if (eval < -Mate + 1000)
      eval += ply;

    if (entry->flag == EXACT)
      return eval;
    if (entry->flag == LOWER && eval >= b)
      return eval;
    if (entry->flag == UPPER && eval <= a)
      return eval;
  }
  
  // qsearch
  if (depth <= 0) {
    return qsearch(pos, a, b, nodes, TT, ply);
  }

  // all legal moves
  std::vector<Move> moves;
  generateMoves<false, false>(pos, moves);

  if (moves.empty()) {
    if (inCheck(pos))
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
  Eval oA = a;

  // the future best evaluation and best move
  Eval Best = -INF;

  Move bestMove;

  for (int i = 0; i < (int)moves.size(); i++) {

    if (!is_running) break;

    pickMove(moves, i, ttMove);

    const Move move = moves[i];

    // play move
    doMove(pos, move);

    // we run the recursively search

    Eval eval;

    if (depth >= 3 && i >= 4 && getType(move.Captured) == NONE && !inCheck(pos)) {

      // LMR (Late Move Reductions)
      Depth r = (_log2(depth) * _log2(i + 1)) / 2;
      r = std::min((int)r, 2);

      eval = -search(depth - 1 - r, pos, -a - 1, -a, nodes, TT, ply + 1);

      if (eval > a) {
        eval = -search(depth - 1, pos, -b, -a, nodes, TT, ply + 1);
      }

    } else {

      // PVS (Principal Variation Search)
      if (i == 0) {
        eval = -search(depth - 1, pos, -b, -a, nodes, TT, ply + 1);
      } else {

        eval = -search(depth - 1, pos, -a - 1, -a, nodes, TT, ply + 1);
        if (eval > a) {
          eval = -search(depth - 1, pos, -b, -a, nodes, TT, ply + 1);
        }

      }

    }

    // undo move
    undoMove(pos, move);

    // okay, maybe we have a new best move

    if (Best < eval) {
      Best = eval;
      bestMove = move;
    }

    a = std::max(a, eval);

    // beta cutoff
    if (eval >= b)
      break;
  }

  Bound flag;

  if (Best <= oA)
    flag = UPPER;
  else if (Best >= b)
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

std::pair<Move, Eval> search_root(Position pos, Depth depth, Eval alpha, Eval beta, int64_t& nodes, tt* TT) {

  // the search is running now
  is_running = true;

  // original alpha
  Eval oA = alpha;

  // we will look at every legal move in the position
  std::vector<Move> moves;
  generateMoves<false, false>(pos, moves);

  // if it is already mate or stalemate
  if (moves.empty()) {
    return {Move(), (inCheck(pos) ? -Mate : 0)};
  }

  // if we can use TT
  Entry* entry = TT->probe(pos.ZobristHash);

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

    pickMove(moves, i, ttMove);

    // don't forget to move
    doMove(pos, moves[i]);

    // we evaluate current position
    Eval eval = -search(depth - 1, pos, -beta, -alpha, nodes, TT, 0);

    // don't forget to undo this move
    undoMove(pos, moves[i]);

    // if we better than all previous moves
    if (eval > BestEval) {
      BestMove = moves[i];
      BestEval = eval;
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

// soft/hard bounds for time management
std::pair<Move, int64_t> iterative_depening(Position pos, tt* TT, Depth maxDepth, long soft, long hard) {

  // start time
  long start = std::chrono::high_resolution_clock::now().time_since_epoch().count();
  long end = start;

  Depth depth = 1;

  // we will return this
  std::vector<Move> temp;
  generateMoves<false, true>(pos, temp);

  if (temp.empty()) {
    return {Move(), 0};
  }

  Move BestMove = temp[0];

  Eval eval = -INF;

  int64_t nodes = 0;

  stop = false;

  do {

    // new nodes counter
    int64_t nnodes = 0;

    is_running = true;

    std::future<std::pair<Move, Eval>> result = std::async(search_root, pos, depth, -INF, INF, std::ref(nnodes), TT);

    while (true) {

      long elapsed = (end - start) / 1'000'000;

      if (!is_running) {
        break;
      } else if (stop && depth > 1) {
        break;
      } else if (elapsed >= hard && depth > 1) {
        break;
      }

      end = std::chrono::high_resolution_clock::now().time_since_epoch().count();
      std::this_thread::sleep_for(std::chrono::milliseconds(1));

    }

    // time is up  or stop command received
    if ((is_running || stop) && depth > 1) {
      is_running = false;
      break;
    }

    auto [bm, neval] = result.get();
    BestMove = bm; eval = neval;

    nodes = nnodes;

    end = std::chrono::high_resolution_clock::now().time_since_epoch().count();

    long Time = (end - start) / 1'000'000;
    int64_t nps = (Time > 0) ? (nodes * 1000 / Time) : 0;

    // UCI

    std::cout << "info  depth " << toLen(depth, 4)
              << " nodes " << toLen(nodes, 11)
              << " nps " << toLen(nps, 9)
              << " time " << toLen(Time, 8)
              << " score " << score(eval)
              << " pv "
              // it will print best move
              << convertMoveToString(BestMove)
              << std::endl;

    // std::cerr << "info  depth " << toLen(depth, 4)
    //           << " nodes " << toLen(nodes, 11)
    //           << " nps " << toLen(nps, 9)
    //           << " time " << toLen(Time, 8)
    //           << " score " << score(eval)
    //           << " pv "
    //           << convertMoveToString(BestMove)
    //           << std::endl;

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
