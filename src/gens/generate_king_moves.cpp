// not a part of engine

// this file will generate king moves from every square

#include <vector>
#include <iostream>

std::vector<std::pair<int, int>> to = {
  {-1, -1},
  {-1, 0},
  {-1, 1},
  {0, -1},
  {0, 1},
  {1, -1},
  {1, 0},
  {1, 1}
};

unsigned long long gen(int r, int f) {
  unsigned long long ans = 0;
  for (auto [x, y] : to) {
    if (r + x >= 0 && r + x < 8 && f + y >= 0 && f + y < 8) {
      ans |= 1ull << ((r + x) * 8 + f + y);
    }
  }
  return ans;
}

int main() {
  std::cout << "= {\n";
  for (int r = 0; r < 8; r++) {
    for (int f = 0; f < 8; f++) {
      std::cout << gen(r, f) << "ULL" << (r == 7 && f == 7 ? "" : ", ");
    }
    std::cout << '\n';
  }
  std::cout << "};\n";
}