#include "nnue.h"
#include "board.h"
#include "bits.h"

void initAccumulator(Accumulator& acc, const Network* nnue, const Position& pos) {
  for (int i = 0; i < HIDDEN; i++) {
    acc.hidden[i] = nnue->B1[i];
  }

  Bitboard pieces = pos.WhitePieces | pos.BlackPieces;
  for (Square S = FirstBit(pieces); S < 64; S = NextBit(pieces, S)) {
    Piece p = pos.getPiece(S);

    int f = featureIndex(p, S);

    addFeature(acc, nnue, f);
  }
}
