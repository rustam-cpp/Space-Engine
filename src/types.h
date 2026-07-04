#pragma once

#include "stdint.h"

using Bitboard = uint64_t;
using Piece = uint8_t;
using Square = uint8_t;
using File = uint8_t;
using Rank = uint8_t;
using Depth = int8_t;
using Eval = int32_t;
using Time = int64_t;
// Castling availability - 0bkqKQ
using CastlingMask = uint8_t;
constexpr Square NullSquare = 255;

constexpr Bitboard FileA        = 0x0101010101010101;
constexpr Bitboard Rank1        = 0x00000000000000FF;
// a bitboard for calculating how good is king safety
constexpr Bitboard Shield       = 0x0000000000000101;
// a bitboard for understanding is the king in the castle region
constexpr Bitboard CastleRegion = 0x00000000000000C7;

enum Color {
  BLACK = 1,
  WHITE = 2
};

enum PieceType {
  NONE, // No Piece
  PAWN,
  KNIGHT,
  BISHOP,
  ROOK,
  QUEEN,
  KING,
  FULL // 0b111
};

enum Bound {
  EXACT,
  LOWER,
  UPPER
};