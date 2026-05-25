#include "evaluation.h"
#include "bits.h"
#include "converts.h"
#include "types.h"

template <PieceType T>
Eval evalPieceType(const Position& pos) {
  Eval eval = 0;
  eval += Count(pos.pieces[makePiece(WHITE, T)]);
  eval -= Count(pos.pieces[makePiece(BLACK, T)]);
  return eval * simp[T];
}

Eval evalKingsInEngame(const Position& pos, Eval Material) {
  Square WhiteK = FirstBit(pos.pieces[makePiece(WHITE, KING)]);
  Square BlackK = FirstBit(pos.pieces[makePiece(BLACK, KING)]);
  int distW = std::max(3 - (int)getFile(WhiteK), (int)getFile(WhiteK) - 4) +
              std::max(3 - (int)getRank(WhiteK), (int)getRank(WhiteK) - 4);
  int distB = std::max(3 - (int)getFile(BlackK), (int)getFile(BlackK) - 4) +
              std::max(3 - (int)getRank(BlackK), (int)getRank(BlackK) - 4);
  int distKings = abs((int)getFile(WhiteK) - (int)getFile(BlackK)) +
                  abs((int)getRank(WhiteK) - (int)getRank(BlackK));
  return (47 * (Material > 0 ? distB : distW) + 16 * (14 - distKings)) * 7 / 10;
}

template <PieceType T>
Eval evalPieceSquareTable(const Position& pos, Eval Material) {
  Eval eval = 0;
  Eval eval2 = 0;
  if (T == KING) {
    Square WhiteK = FirstBit(pos.pieces[makePiece(WHITE, KING)]);
    Square BlackK = FirstBit(pos.pieces[makePiece(BLACK, KING)]);
    Bitboard WkingShL = 0; // left
    Bitboard WkingShM = 0; // middle
    Bitboard WkingShR = 0; // right
    // Square + 7 is [rank, file] -> [rank + 1, file - 1]
    if (getFile(WhiteK) > 0) WkingShL |= Shield << (WhiteK + 7);
    WkingShM |= Shield << (WhiteK + 8);
    // Square + 9 is [rank, file] -> [rank + 1, file + 1]
    if (getFile(WhiteK) < 7) WkingShR |= Shield << (WhiteK + 9);
    Bitboard BkingShL = 0; // left
    Bitboard BkingShM = 0; // middle
    Bitboard BkingShR = 0; // right
    // Square - 17 is [rank, file] -> [rank - 2, file - 1]
    if (getFile(BlackK) > 0) BkingShL = Shield << (BlackK - 17);
    BkingShM = Shield << (BlackK - 16);
    // Square - 15 is [rank, file] -> [rank - 2, file + 1]
    if (getFile(BlackK) < 7) BkingShR = Shield << (BlackK - 15);
    // these formulas are made for speed things up
    
    // counting pawns that protect white king
    int cntW = 0;
    cntW += Count(WkingShL & pos.pieces[makePiece(WHITE, PAWN)]) >= 1;
    cntW += Count(WkingShM & pos.pieces[makePiece(WHITE, PAWN)]) >= 1;
    cntW += Count(WkingShR & pos.pieces[makePiece(WHITE, PAWN)]) >= 1;
    // counting pawns that protect black king
    int cntB = 0;
    cntB += Count(BkingShL & pos.pieces[makePiece(BLACK, PAWN)]) >= 1;
    cntB += Count(BkingShM & pos.pieces[makePiece(BLACK, PAWN)]) >= 1;
    cntB += Count(BkingShR & pos.pieces[makePiece(BLACK, PAWN)]) >= 1;

    eval += (cntW * 2 - 5) * 6 * ((((1ULL << WhiteK) & CastleRegion) != 0) * 4 + 1);
    eval -= (cntB * 2 - 5) * 6 * ((((1ULL << BlackK) & (CastleRegion << 7)) != 0) * 4 + 1);

    eval2 = evalKingsInEngame(pos, Material);
  }

  int count = 0;
  // -1 because of white king
  count += Count(pos.WhitePieces ^ pos.pieces[makePiece(WHITE, PAWN)]) - 1;
  // -1 because of black king
  count += Count(pos.BlackPieces ^ pos.pieces[makePiece(BLACK, PAWN)]) - 1;
  // we will use this formula for "softer" evaluation

  Piece wP = makePiece(WHITE, T);
  Piece bP = makePiece(BLACK, T);
  
  // in these 2 fors we are watching at each piece type
  // entry on the board with fast bit-operation functions
  for (Square i = FirstBit(pos.pieces[wP]); i < 64;
              i = NextBit(pos.pieces[wP], i)) {
    if (T == KING || T == PAWN) {
      eval += (Weight[T][i] * count + Weight[T + 6][i] * (14 - count)) / 14;
    } else {
      eval += Weight[T][i];
    }
  }
  for (Square i = FirstBit(pos.pieces[bP]); i < 64;
              i = NextBit(pos.pieces[bP], i)) {
    if (T == KING || T == PAWN) {
      eval -= (Weight[T][swapRank(i)] * count + Weight[T + 6][swapRank(i)] * (14 - count)) / 14;
    } else {
      eval -= Weight[T][swapRank(i)];
    }
  }

  // of middle game and end game for kings
  if (T == KING) {
    return (eval * count * count + eval2 * (196 - count * count)) / 196;
  } else {
    return eval;
  }
}

template <Color C>
Eval evalPawns(const Position& pos) {
  Eval eval = 0;
  Piece P = makePiece(C, PAWN);
  // opposite color Pawn
  Piece oP = swapColor(P);
  for (Square i = FirstBit(pos.pieces[P]); i < 64;
              i = NextBit(pos.pieces[P], i)) {
    // Mask is a mix of rank on the left (if exists),
    // rank on the right (if exists) and middle rank.
    Bitboard Mask = 0;
    Bitboard Middle = FileA << getFile(i);
    // left rank
    if (getFile(i) > 0) {
      Mask |= FileA << (getFile(i) - 1);
    }
    // middle rank
    Mask |= Middle;
    // right rank
    if (getFile(i) < 7) {
      Mask |= FileA << (getFile(i) + 1);
    }
    // with this bits operations we will calculate is pawn
    // passed, isolated, blocked, protected, doubled
    Bitboard Prev = Mask >> (8 * (8 - getRank(i)));
    Bitboard Next = Mask << (8 * (getRank(i) + 1));
    Bitboard PrevM = Middle >> (8 * (8 - getRank(i)));
    Bitboard NextM = Middle << (8 * (getRank(i) + 1));
    if (C == BLACK) {
      // if current pawns are black then them next squares
      // are previous for them and previous are next
      std::swap(Prev, Next);
      std::swap(PrevM, NextM);
    }
    int cur = 0;
    if ((pos.pieces[oP] & Next) == 0ULL) {
      // passed pawn
      cur += Weight[7][(C == WHITE ? i : swapRank(i))];
      if ((pos.pieces[P] & (Prev ^ PrevM)) != 0ULL) {
        // protected
        cur += 30;
      }
    }
    if ((pos.pieces[P] & (Prev ^ PrevM)) == 0ULL) {
      // isolated pawn
      cur -= 25;
    }
    if ((pos.pieces[P] & NextM) != 0ULL) {
      // doubled
      cur -= 40;
    }
    eval += cur;
  }
  return eval;
}

Eval evaluation(const Position& pos) {
  Eval eval = 0;
  // material
  Eval material = 0;
  material += evalPieceType<PAWN>(pos);
  material += evalPieceType<KNIGHT>(pos);
  material += evalPieceType<BISHOP>(pos);
  material += evalPieceType<ROOK>(pos);
  material += evalPieceType<QUEEN>(pos);
  eval += material;
  // piece square tables
  eval += evalPieceSquareTable<PAWN>(pos, material);
  eval += evalPieceSquareTable<KNIGHT>(pos, material);
  eval += evalPieceSquareTable<BISHOP>(pos, material);
  eval += evalPieceSquareTable<ROOK>(pos, material);
  eval += evalPieceSquareTable<QUEEN>(pos, material);
  eval += evalPieceSquareTable<KING>(pos, material);
  // special evaluation for pawn structure
  eval += evalPawns<WHITE>(pos);
  eval -= evalPawns<BLACK>(pos);
  // position good for white is bad for black
  int mul = (pos.WhiteToMove ? 1 : -1);
  // final evaluation
  return eval * mul;
}
