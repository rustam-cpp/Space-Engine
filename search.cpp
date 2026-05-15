#include "search.h"
#include "constants.h"
#include "converts.h"
#include "movegen.h"
#include "evaluation.h"
#include "types.h"
#include <algorithm>
#include <chrono>
#include <future>
#include <thread>

std::atomic<bool> is_running = false;

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

// first we will search good moves
inline bool cmp(const Move& m1, const Move& m2) {
  return getScore(m1) > getScore(m2);
}

Eval qsearch(Position pos, Eval a, Eval b, int64_t& nodes, Depth ply) {

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

  a = std::max(a, eval);

  std::vector<Move> moves;
  generateMoves<true, false>(pos, moves);

  if (moves.empty()) {
    generateMoves<false, true>(pos, moves);
    // the end
    if (moves.empty()) {
      if (inCheckOp(pos))
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
  } else {
    std::sort(moves.begin(), moves.end(), cmp);
  }

  for (const Move& move : moves) {

    if (!is_running) break;

    doMove(pos, move);

    eval = -qsearch(pos, -b, -a, nodes, ply + 1);

    undoMove(pos, move);

    if (eval >= b) {
      return eval;
    }

    a = std::max(a, eval);
  }

  return a;

}

Eval search(Depth depth, Position pos, Eval a, Eval b, int64_t& nodes, tt* TT, Depth ply) {

  if (!is_running) return 0;

  nodes++;

  // if there is a draw on the board, then you need to return 0
  if (pos.isRepetitionDraw() || pos.isFiftyMoveDraw())
    return 0;
  
  // qsearch
  if (depth <= 0) {
    return qsearch(pos, a, b, nodes, ply);
  }

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

  // all legal moves
  std::vector<Move> moves;
  generateMoves<false, false>(pos, moves);

  if (moves.empty()) {
    if (inCheckOp(pos))
      // checkmate on the board
      return -Mate + ply;
    else
      // stalemate on the board
      return 0;
  } else {
    // but because of we have alpha-beta pruning
    // we can look at good moves first
    std::sort(moves.begin(), moves.end(), cmp);
  }

  // original alpha
  Eval oA = a;

  // the future best evaluation and best move
  Eval Best = -INF;

  for (int i = 0; i < (int)moves.size(); i++) {

    const Move move = moves[i];

    if (!is_running) break;

    // play move
    doMove(pos, move);

    // we run the recursively search
    
    Eval eval = -search(depth - 1, pos, -b, -a, nodes, TT, ply + 1);
    
    // undo move
    undoMove(pos, move);

    // okay, maybe we have a new best move

    if (Best < eval) {
      Best = eval;
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

  TT->store(pos.ZobristHash, depth, sE, flag);

  return Best;

}

// a good random
std::mt19937 rng(std::chrono::high_resolution_clock::now().time_since_epoch().count());

std::pair<Move, Eval> search_root(Position pos, Depth depth, Eval alpha, Eval beta, int64_t& nodes, tt* TT) {

  // the search is running now
  is_running = true;

  // we will look at every legal move in the position
  std::vector<Move> moves;
  generateMoves<false, false>(pos, moves);

  // this array will contain all moves, that have the
  // same evaluation as the best (they are all the best)
  std::vector<Move> Bests;
  
  // and this is the best move evaluation
  Eval BestEval = -INF;

  for (const Move& move : moves) {

    if (!is_running) break;

    // don't forget to move
    doMove(pos, move);

    // we evaluate current position
    Eval eval = -search(depth - 1, pos, -beta, -alpha, nodes, TT, 0);

    // if we better than all previous moves
    if (eval > BestEval) {
      Bests.clear();
      BestEval = eval;
    // or it's another option, that not worse, than previous
    }
    if (eval == BestEval) {
      Bests.push_back(move);
    }

    // std::cerr << convertMoveToString(move) << ": " << eval << std::endl;

    // don't forget to undo this move
    undoMove(pos, move);
  }

  // now we are not running
  is_running = false;

  // if it is already mate or stalemate
  if (Bests.empty()) {
    return {Move(), (inCheck(pos) ? -Mate : 0)};
  }

  // we peak a random move from all the best
  int i = std::uniform_int_distribution<int>(0, (int)Bests.size() - 1)(rng);

  // and return values
  return {Bests[i], BestEval};
}

// soft/hard bounds for time management
std::pair<Move, int64_t> iterative_depening(Position pos, tt* TT, Depth maxDepth, long soft, long hard) {

  Depth depth = 1;

  // we will return this
  Move BestMove = Move();

  Eval eval = -INF;

  // start time
  long start = std::chrono::high_resolution_clock::now().time_since_epoch().count();
  long end = start;

  // previous time
  long prevTime = 0;

  int64_t nodes = 0;

  do {

    // new nodes counter
    int64_t nnodes = 0;

    is_running = true;

    std::future<std::pair<Move, Eval>> result = std::async(search_root, pos, depth, -INF, INF, std::ref(nnodes), TT);
    while (is_running && (depth == 1 || (end - start) / 1'000'000 <= hard)) {
      end = std::chrono::high_resolution_clock::now().time_since_epoch().count();
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // time is up
    if (is_running) {
      is_running = false;
      break;
    }
    
    auto [bm, neval] = result.get();
    BestMove = bm; eval = neval;
    
    nodes = nnodes;
    
    end = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    
    int Time = (end - start) / 1'000'000;
    
    int64_t elapsed = Time - prevTime;
    int64_t nps = (elapsed > 0) ? (nodes * 1000 / elapsed) : 0;

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
    
    prevTime = Time;

    // if we potentially will time out
    if (Time > soft) break;

  } while (depth <= maxDepth);

  return {BestMove, nodes};

}