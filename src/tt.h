#pragma once

#include "types.h"
#include "constants.h"
#include "memory.h"
#include "move.h"

// a struct for an element of transposition table (TT)
struct TTentry {
  // evaluation
  Eval eval;
  // zobrist hash
  uint64_t zhash;
  // depth we reached
  Depth depth;
  // flag
  Bound flag;
  // tt best move
  Move bestMove;
  // constructor
  TTentry() {
    zhash = 0;
    eval = -INF;
    depth = -1;
    flag = EXACT;
  }
};

// ZH - zobrist hash

struct tt {
  // all the elements
  TTentry* table;
  // size of this array
  int size;
  // constructor
  tt(int sizeMB) {

    // if sizeMB = 0
    sizeMB = std::max(sizeMB, 1);
    
    // calculating size of array
    size = 1048576LL * sizeMB / sizeof(TTentry);
    size = 1 << (31 - __builtin_clz(size));

    // creating table
    table = new TTentry[size];
    std::fill(table, table + size, TTentry());
  }

  inline TTentry* probe(uint64_t ZH);
  
  inline void store(uint64_t ZH, Depth d, Eval e, Bound f, const Move& m);
  
  void clear();
  // destructor
  ~tt() {
    delete[] table;
  }
};

inline TTentry* tt::probe(uint64_t ZH) {
  TTentry& entry = table[ZH & (size - 1)];
  if (entry.zhash == ZH) {
    return &entry;
  }
  return nullptr;
}

inline void tt::store(uint64_t ZH, Depth d, Eval e, Bound f, const Move& m) {
  // getting current entry
  TTentry& entry = table[ZH & (size - 1)];

  // check, that we can store or replace it
  if (entry.zhash == 0 || entry.zhash == ZH || d >= entry.depth) {
    entry.zhash = ZH;
    entry.depth = d;
    entry.eval = e;
    entry.flag = f;
    entry.bestMove = m;
  }
}