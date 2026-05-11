#include "tt.h"

void tt::clear() {
  std::fill(table, table + size, Entry());
}