#include "converts.h"
#include <iostream>

void outBb(Bitboard bb) {
  int rank = 7;
  int file = 0;
  while (rank >= 0) {
    while (file < 8) {
      if (bb & (1ULL << makeSquare(rank, file))) {
        std::cout << 1;
      } else {
        std::cout << 0;
      }
      file++;
    }
    std::cout << '\n';
    file = 0;
    rank--;
  }
}

std::string toLen(int64_t x, int len) {
  std::string ans = std::to_string(x);
  while ((int)ans.size() < len)
    ans += ' ';
  return ans;
}