// not a part of engine

// this file will generate an array
// int mvv_lva[7][7]

#include <iostream>

const int value[7] = {
  0, 100, 300, 310, 500, 900, 100000
};

int main() {
  std::cout << "= {\n";
  for (int v = 0; v < 7; v++) {
    std::cout << "  { ";
    for (int a = 0; a < 7; a++) {
      std::cout << value[v] * 10 - value[a] << (a < 6 ? ", " : "");
    }
    std::cout << " }" << (v < 6 ? "," : "") << '\n';
  }
  std::cout << "}\n";
}