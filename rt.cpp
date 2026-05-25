#include "rt.h"

bool rt::isDraw(uint64_t ZH, int hm) {

  if (count[ZH & 4095] < 3) {
    return false;
  }

  int cnt = 0;

  // we will scan moves after last capture or pawn move
  int start = (int)stack.size() - hm;
  start = std::max(start, 0);

  for (int i = (int)stack.size() - 1; i >= start; i -= 2) {

    if (stack[i] == ZH)
      cnt++;

    if (cnt >= 3)
      return true;

  }

  return false;

}

void rt::add(uint64_t ZH) {
  stack.push_back(ZH);
  count[ZH & 4095]++;
}

void rt::del() {
  count[stack.back() & 4095]--;
  stack.pop_back();
}

void rt::clear() {
  stack.clear();
  stack.shrink_to_fit();
  for (int i = 0; i < 4096; i++) {
    count[i] = 0;
  }
}
