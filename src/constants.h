#pragma once

#include "stdint.h"
#include "types.h"
#include <string>
#include <vector>

const Eval Mate = 2'000'000'000;

// the evaluation we can't reach
const Eval INF = 2'147'483'647;
const int64_t BIG_INF = 9'000'000'000'000'000'000;

const std::string StartFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

const Depth MAX_PLY = 64;

const std::vector<std::pair<std::string, Depth>> benchPositions = {
  {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 6},
  {"r1bqk2r/ppp2ppp/2np1n2/2b1p3/2B1P3/3P1N2/PPP2PPP/RNBQ1RK1 w kq - 0 6", 6},
  {"r2q1rk1/1ppn1pp1/2npb2p/pBb1p3/4P3/2PP1N1P/PP1N1PP1/R1BQR1K1 w - - 4 11", 6},
  {"r3r1k1/2p2pp1/1nN4p/p1b3q1/4P3/2P1N2b/PPQ2PP1/R1B1R1K1 w - - 1 19", 6},
  {"1r2r1k1/2p3p1/1b2b1qp/8/p1nNNB2/2P2P2/PP3QP1/1R2R1K1 w - - 1 27", 6},
  {"6k1/2B3p1/7p/8/p3R3/2P2P2/P4KP1/1r1n4 w - - 1 35", 8},
  {"6n1/1r4pk/3R3p/4B3/5PK1/8/6P1/8 w - - 7 43", 6},
  {"8/6pk/8/1r2B2K/2n2P2/R7/6P1/8 w - - 3 51", 7},
  {"6k1/6p1/r7/6K1/8/6R1/6P1/8 b - - 9 58", 7},
  {"8/6p1/r5k1/8/6P1/6KR/8/8 b - - 6 64", 7},
  {"8/8/6p1/6K1/6P1/7k/8/8 b - - 7 72", 15},
  {"8/8/3K4/5k2/8/8/8/8 b - - 12 80", 12},
  {"4k3/8/8/8/8/8/4P3/4K3 w - - 0 1", 12},
  {"3rr1k1/1p1q1pp1/1bp4p/p2pP1nN/3P4/PP4P1/1BR2PPK/3QR3 w - - 0 28", 6},
  {"3rr1k1/1pb3p1/2p1q2p/p2pp3/P2P1PP1/1P3QK1/1B2R1P1/5R2 w - - 0 36", 6},
  {"6k1/2b1r3/2p1q2p/pp2P3/P3RpP1/1P3Q2/1B4PK/8 w - - 2 44", 6},
  {"4k3/8/7p/pp1qPQb1/3B2P1/1P6/6P1/7K w - - 7 52", 6},
  {"4k3/4b3/4P2p/8/8/B7/6P1/7K w - - 1 60", 8}
};

constexpr Eval mvv_lva[7][7] = {
  { 0, -100, -300, -310, -500, -900, -100000 },
  { 1000, 900, 700, 690, 500, 100, -99000 },
  { 3000, 2900, 2700, 2690, 2500, 2100, -97000 },
  { 3100, 3000, 2800, 2790, 2600, 2200, -96900 },
  { 5000, 4900, 4700, 4690, 4500, 4100, -95000 },
  { 9000, 8900, 8700, 8690, 8500, 8100, -91000 },
  { 1000000, 999900, 999700, 999690, 999500, 999100, 900000 }
};