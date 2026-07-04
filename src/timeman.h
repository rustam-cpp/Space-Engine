#pragma once

#include "types.h"

inline Time soft_bound(Time time, Time inc, int movesToGo) {
  return time / (movesToGo * 7 / 4) + inc * 2 / 5;
}

inline Time hard_bound(Time time, Time inc, int movesToGo) {
  return time / movesToGo + inc / 2;
}

inline Time soft_bound_fixed_movetime(Time time) {
  return time * 4 / 7;
}