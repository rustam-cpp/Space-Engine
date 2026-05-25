#pragma once

#include "types.h"
#include <vector>

struct rt {

  std::vector<uint64_t> stack;
  int count[4096];

  rt() {
    for (int i = 0; i < 4096; i++) {
      count[i] = 0;
    }
  }

  bool isDraw(uint64_t ZH, int hm);

  void add(uint64_t ZH);
  void del();

  void clear();

};