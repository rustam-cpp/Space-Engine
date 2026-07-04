#pragma once

inline long soft_bound(long time, long inc, int movesToGo) {
  return time / (movesToGo * 7 / 4) + inc * 2 / 5;
}

inline long hard_bound(long time, long inc, int movesToGo) {
  return time / movesToGo + inc / 2;
}

inline long soft_bound_fixed_movetime(long time) {
  return time * 4 / 7;
}