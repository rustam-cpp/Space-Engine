#pragma once

inline long soft_bound(long time, long inc) {
  return time / 70 + inc * 2 / 5;
}

inline long hard_bound(long time, long inc) {
  return time / 40 + inc / 2;
}