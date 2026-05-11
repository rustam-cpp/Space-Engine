#pragma once

#include "types.h"
#include "board.h"

struct Move {
  Move() {
    From = 0, To = 0;
    Moved = NONE, Captured = NONE, PromotedTo = NONE;
    Mask = 0;
    Castling = false; EnPassant = false;
    EnPassantSquare = 0;
    HalfmoveClock = 0;
  }
  // Move info
  Square From, To;
  Piece Moved, Captured, PromotedTo;
  bool Castling;
  bool EnPassant;
  // Previous values
  CastlingMask Mask;
  Square EnPassantSquare;
  uint8_t HalfmoveClock;
  bool operator==(const Move&) const = default;
};

Move makeMove(const Position& pos, Square From, Square To, Piece PromotedTo);
void doMove(Position& pos, const Move& move);
void undoMove(Position& pos, const Move& move);

uint64_t hashMove(const Move& move);