// not a part of engine

// this file will generate an array
// int zobrist[32][64]

#include <iostream>
#include <random>

std::mt19937_64 rng(42);

int main() {
  std::cout << "= {\n";
  for (int p = 0; p < 32; p++) {
    for (int s = 0; s < 64; s++) {
      std::cout << rng() << "ULL";
      if (s < 63) std::cout << ", ";
    }
    std::cout << " }";
    if (p < 31) std::cout << ',';
    std::cout << '\n';
  }
  std::cout << "};\n";
}