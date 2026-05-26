#include "tt.h"

void tt::clear() {
  occupied = 0;
  std::fill(table, table + size, TTentry());
}