#pragma once

// tricks used in search:
//  * alpha-beta pruning
//  * move ordering
//  * iterative deepening
//  * aspiration window
//  * transposition table

#include "movegen.h"
#include "tt.h"
#include <random>
#include <chrono>

// debug function, that finds the count of all
// leafs in search tree on a givved depth depth
int64_t perft(Depth depth, Position pos);

// returns the score how good or bad the move is
inline Eval getScore(const Move& move);

void pickMove(std::vector<Move>& moves, int start, const Move& ttMove);

// we cannot use static evaluation if it's a capture
// or a pawn promotion available in position, so
// we run another search that will iterate over
// all captures and pawn promotions
Eval qsearch(Position pos, Eval a, Eval b, int64_t& nodes, Depth ply);

// a search function created for calculating over
// ~all moves, positions to calcualte the best move
Eval search(Depth depth, Position pos, Eval a, Eval b, int64_t& nodes, tt* TT, Depth ply);

// finds the best move in the position
std::pair<Move, Eval> search_root(Position pos, Depth depth, Eval alpha, Eval beta, int64_t& nodes, tt* TT);

// we will use iterative deepening
std::pair<Move, int64_t> iterative_depening(Position pos, tt* TT, Depth maxDepth, long soft, long hard);

// stops search when "stop" command received
void stopCommand();