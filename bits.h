#pragma once

#include "types.h"

// find's first bit in 64-bit number
inline Square FirstBit(Bitboard B) {
  if (B == 0) return 64;
  return __builtin_ctzll(B);
}

// find's first bit after xth one
inline Square NextBit(Bitboard B, Square i) {
  if (i >= 63) return 64;
  if ((B >> (i + 1)) == 0) return 64;
  else return i + 1 + __builtin_ctzll(B >> (i + 1));
}

// find's count of bits (1)
inline uint8_t Count(Bitboard B) {
  return __builtin_popcountll(B);
}