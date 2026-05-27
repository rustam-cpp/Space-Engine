#pragma once

#include "types.h"

// find's the first bit in 64-bit number
inline Square FirstBit(Bitboard B) {
  if (B == 0) return 64;
  return __builtin_ctzll(B);
}

// find's the last bit in 64-bit number
inline uint8_t LastBit(Bitboard B) {
  if (B == 0) return 64;
  return 63 - __builtin_clzll(B);
}

// find's the first bit after xth one
inline Square NextBit(Bitboard B, Square i) {
  if (i >= 63) return 64;
  if ((B >> (i + 1)) == 0) return 64;
  else return i + 1 + __builtin_ctzll(B >> (i + 1));
}

// find's count of bits (1)
inline uint8_t Count(Bitboard B) {
  return __builtin_popcountll(B);
}

inline uint8_t _log2(int x) {
  return 31 - __builtin_clz(x);
}