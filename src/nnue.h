#pragma once

#include "types.h"
#include <string>

inline Color gc(Piece P) {
  if (P & (4 << BLACK)) return BLACK;
  return WHITE;
}

inline PieceType gt(Piece P) {
  return PieceType(P & FULL);
}

inline int featureIndex(Piece P, Square S) {
  return ((gc(P) - 1) * 6 + (gt(P) - 1)) * 64 + S;
}

constexpr int INPUT = 772;
constexpr int HIDDEN = 128;

struct Network {

  // input
  int16_t W1[INPUT][HIDDEN];
  int16_t B1[HIDDEN];

  // hidden layer
  int16_t W2[HIDDEN];
  int16_t B2;

};

struct Accumulator {

  int32_t hidden[HIDDEN];

};

struct Position;

inline void addFeature(Accumulator& acc, const Network* nnue, int featureIdx) {
  int32_t* h = acc.hidden;

  for (int i = 0; i < HIDDEN; i++) {
    h[i] += nnue->W1[featureIdx][i];
  }
}

inline void delFeature(Accumulator& acc, const Network* nnue, int featureIdx) {
  int32_t* h = acc.hidden;

  for (int i = 0; i < HIDDEN; i++) {
    h[i] -= nnue->W1[featureIdx][i];
  }
}

void initAccumulator(Accumulator& acc, const Network* nnue, const Position& pos);

inline int32_t evaluate(const Accumulator& acc, const Network* nnue) {
  const int32_t* h = acc.hidden;

  int32_t eval = nnue->B2;

  for (int i = 0; i < HIDDEN; i++) {
    int16_t x = h[i];

    if (x < 0) x = 0;

    eval += x * nnue->W2[i];
  }

  return eval;
}
