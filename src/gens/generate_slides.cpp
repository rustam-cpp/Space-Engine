// not a part of engine

// this file will generate an array
// int slide[8][8][64]

#include <vector>
#include <iostream>

std::vector<std::pair<int, int>> dir = {
  {-1, -1},
  {-1, 1},
  {1, -1},
  {1, 1},
  {-1, 0},
  {0, -1},
  {0, 1},
  {1, 0},
};

unsigned long long slide(int d, int l, int s) {
  unsigned long long ans = 0;
  int r = s/8;
  int f = s%8;
  int x = dir[d].first;
  int y = dir[d].second;
  while (l--) {
    if (r+x>=0 && r+x<8 && f+y>=0 && f+y<8) {
      r += x;
      f += y;
    } else {
      break;
    }
    ans |= 1ull << (r*8 + f);
  }
  return ans;
}

int main() {
  std::cout << "= {\n";
  for (int d = 0; d < 8; d++) {
    std::cout << "  {\n";
    for (int l = 0; l < 8; l++) {
      std::cout << "    { ";
      for (int s = 0; s < 64; s++) {
        std::cout << slide(d, l, s) << "ULL";
        if (s < 63) std::cout << ", ";
      }
      std::cout << " }";
      if (l < 7) std::cout << ',';
      std::cout << '\n';
    }
    std::cout << "  }";
    if (d < 7) std::cout << ',';
    std::cout << '\n';
  }
  std::cout << "};\n";
}