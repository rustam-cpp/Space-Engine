#include "uci.h"
#include "constants.h"
#include "converts.h"
#include "search.h"
#include "timeman.h"

std::vector<std::string> split(std::string command) {
  // the final result
  std::vector<std::string> result;
  for (char c : command) {
    // if it's the first word or a new word
    if (result.empty() || (c == ' ' && result.back() != "")) {
      // we create a new string
      result.emplace_back();
    } else if (result.back() != "" || c != ' ') {
      // else we add char to last word
      result.back() += c;
    }
  }
  return result;
}

void processPositionCommand(Position* pos, rt* RT, std::string command) {
  std::vector<std::string> cmd = split(command);
  int i;
  if (cmd[1] == "startpos") {
    pos->convertFromFen(StartFen, RT);
    i = 2;
  } else {
    std::string fen = cmd[2] + " "
                    + cmd[3] + " "
                    + cmd[4] + " "
                    + cmd[5] + " "
                    + cmd[6] + " "
                    + cmd[7];
    pos->convertFromFen(fen, RT);
    i = 8;
  }
  if (i < (int)cmd.size() && cmd[i] == "moves") {
    // we parse all moves from the command
    while (++i < (int)cmd.size()) {
      Square From, To;
      Piece PromotedTo = NONE;
      From = convertStringToSquare(cmd[i].substr(0, 2));
      To = convertStringToSquare(cmd[i].substr(2, 2));
      if (cmd[i].size() == 5) {
        PromotedTo = convertStringToPiece(cmd[i][4]);
        // we need this if because all promoted pieces in uci are black.
        // for example e7e8q, but e7 is a white pawn
        if (pos->WhiteToMove) PromotedTo = swapColor(PromotedTo);
      }
      doMove(pos, makeMove(pos, From, To, PromotedTo), RT);
    }
  }
}

void processGoCommand(Position* pos, tt* TT, rt* RT, std::string command) {
  Depth maxDepth = MAX_PLY;
  // base time, increment
  Time myTime = BIG_INF, myInc = 0;
  Time moveTime = BIG_INF;
  int movesToGo = 40;
  bool infinite = false;
  std::vector<std::string> cmd = split(command);
  for (int i = 0; i < (int)cmd.size(); i++) {
    // base time
    if (cmd[i] == "wtime" && pos->WhiteToMove) {
      if ((int)cmd.size() >= i)
        myTime = stol(cmd[i+1]);
    } else if (cmd[i] == "btime" && !pos->WhiteToMove) {
      if ((int)cmd.size() >= i)
        myTime = stol(cmd[i+1]);
    }
    // increment
    if (cmd[i] == "winc" && pos->WhiteToMove) {
      if ((int)cmd.size() >= i)
        myInc = stol(cmd[i+1]);
    } else if (cmd[i] == "binc" && !pos->WhiteToMove) {
      if ((int)cmd.size() >= i)
        myInc = stol(cmd[i+1]);
    }
    // max depth
    if (cmd[i] == "depth") {
      if ((int)cmd.size() >= i)
        maxDepth = stol(cmd[i+1]);
    }
    // move time
    if (cmd[i] == "movetime") {
      if ((int)cmd.size() >= i)
        moveTime = stol(cmd[i+1]);
    }
    // moves to go
    if (cmd[i] == "movestogo") {
      if ((int)cmd.size() >= i)
        movesToGo = stoi(cmd[i+1]);
    }
    // infinite time think
    if (cmd[i] == "infinite") {
      infinite = true;
    }
  }
  if (infinite) {
    moveTime = BIG_INF;
    myTime = BIG_INF;
    myInc = 0;
  }
  Move bestmove;
  if (moveTime == BIG_INF) {
    // BestMove, Nodes
    auto [bm, n] = iterative_depening(pos, TT, RT, maxDepth,
                   soft_bound(myTime, myInc, movesToGo) - 1,
                   hard_bound(myTime, myInc, movesToGo) - 1);
    bestmove = bm;
  } else {
    auto [bm, n] = iterative_depening(pos, TT, RT, maxDepth,
                   soft_bound_fixed_movetime(moveTime) - 1,
                   moveTime - 1);
    bestmove = bm;
  }
  std::cout << "bestmove " << convertMoveToString(bestmove) << std::endl;
}

void processBenchCommand(Position* pos) {
  tt* tempTT = new tt(8);
  rt* tempRT = new rt;
  int64_t nodes = 0;
  Time start = std::chrono::high_resolution_clock::now().time_since_epoch().count();
  for (const auto& [fen, depth] : benchPositions) {
    tempTT->clear();
    tempRT->clear();
    pos->convertFromFen(fen, tempRT);
    nodes += iterative_depening(pos, tempTT, tempRT, depth, BIG_INF, BIG_INF).second;
  }
  Time end = std::chrono::high_resolution_clock::now().time_since_epoch().count();
  Time elapsed = (end - start) / 1'000'000;
  std::cout << nodes << " nodes " << (elapsed > 0 ? nodes * 1000 / elapsed : 0) << " nps" << std::endl;
  delete tempTT;
  delete tempRT;
}

void processPerftTestCommand(Position* pos, bool BIG) {
  rt* tempRT = new rt;
  int passed = 0;
  int failed = 0;
  int tn = 0;
  for (auto [fen, d, ans] : (BIG ? bigPerftPositions : perftPositions)) {
    pos->convertFromFen(fen, tempRT);
    int64_t res = perft(d, d, pos, tempRT);
    if (res == ans) {
      passed++;
      std::cout << "Test " << ++tn << " passed!" << std::endl;
    } else {
      failed++;
      std::cout << "Test " << ++tn << " failed!" << std::endl;
    }
  }
  std::cout << "passed " << passed << "/" << perftPositions.size();
  std::cout << " failed " << failed << std::endl;
  delete tempRT;
}