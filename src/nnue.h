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

constexpr int INPUT = 768;
constexpr int HIDDEN = 128;

struct Network {

  // input
  float W1[INPUT][HIDDEN];
  float B1[HIDDEN];

  // hidden layer
  float W2[HIDDEN];
  float B2;

};

struct Accumulator {

  float hidden[HIDDEN];

};

struct Position;

inline void addFeature(Accumulator& acc, const Network* nnue, int featureIdx) {
  float* h = acc.hidden;

  for (int i = 0; i < HIDDEN; i++) {
    h[i] += nnue->W1[featureIdx][i];
  }
}

inline void delFeature(Accumulator& acc, const Network* nnue, int featureIdx) {
  float* h = acc.hidden;

  for (int i = 0; i < HIDDEN; i++) {
    h[i] -= nnue->W1[featureIdx][i];
  }
}

void initAccumulator(Accumulator& acc, const Network* nnue, const Position& pos);

inline float evaluate(const Accumulator& acc, const Network* nnue) {
  const float* h = acc.hidden;

  float eval = nnue->B2;

  for (int i = 0; i < HIDDEN; i++) {
    float x = h[i];

    x = std::max(x, 0.0f);

    eval += x * nnue->W2[i];
  }

  return eval;
}
