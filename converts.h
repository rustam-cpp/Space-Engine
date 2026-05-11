#pragma once

#include "types.h"
#include "move.h"
#include <string>

// makes piece from color and type
inline Piece makePiece(Color c, PieceType t) {
  return (4 << c) ^ t;
}

inline Color getColor(Piece P) {
  if (P & (4 << BLACK)) return BLACK;
  return WHITE;
}

inline PieceType getType(Piece P) {
  return PieceType(P & FULL);
}

// canges color (black -> white, white -> black) of P
inline Piece swapColor(Piece P) {
  return makePiece(Color(3 - getColor(P)), getType(P));
}

// makes square from rank and file
inline Square makeSquare(Rank r, File f) {
  return r * 8 + f;
}

inline Rank getRank(Square S) {
  return S / 8;
}

inline File getFile(Square S) {
  return S % 8;
}

// converts (r, f) -> (7 - r, f)
inline Square swapRank(Square S) {
  return makeSquare(7 - getRank(S), getFile(S));
}

// converts (r, f) -> coordinates in algebraic notation
inline std::string convertSquareToString(Square S) {
  std::string square;
  square += getFile(S) + 'a';
  square += getRank(S) + '1';
  return square;
}

// converts coordinates in algebraic notation -> (r, f)
inline Square convertStringToSquare(std::string S) {
  return makeSquare(S[1] - '1', S[0] - 'a');
}

// converts pieces in algebraic notation (P or q e.g.)
// to a single number -> (Color | PieceType)
inline Piece convertStringToPiece(char P) {
  if (P == 'p') return makePiece(BLACK, PAWN);
  if (P == 'n') return makePiece(BLACK, KNIGHT);
  if (P == 'b') return makePiece(BLACK, BISHOP);
  if (P == 'r') return makePiece(BLACK, ROOK);
  if (P == 'q') return makePiece(BLACK, QUEEN);
  if (P == 'k') return makePiece(BLACK, KING);
  if (P == 'P') return makePiece(WHITE, PAWN);
  if (P == 'N') return makePiece(WHITE, KNIGHT);
  if (P == 'B') return makePiece(WHITE, BISHOP);
  if (P == 'R') return makePiece(WHITE, ROOK);
  if (P == 'Q') return makePiece(WHITE, QUEEN);
  if (P == 'K') return makePiece(WHITE, KING);
  return NONE;
}

// converts a single number to piece in algeraic notation
inline char convertPieceToString(Piece P) {
  char piece;
  if (getType(P) == PAWN) piece = 'P';
  if (getType(P) == KNIGHT) piece = 'N';
  if (getType(P) == BISHOP) piece = 'B';
  if (getType(P) == ROOK) piece = 'R';
  if (getType(P) == QUEEN) piece = 'Q';
  if (getType(P) == KING) piece = 'K';
  if (getColor(P) == BLACK) piece += 'a' - 'A';
  if (getType(P) == NONE) piece = ' ';
  return piece;
}

// converts struct Move -> e2e4 e.g.
// if it is a null move -> 0000
inline std::string convertMoveToString(Move M) {
  if (M == Move()) {
    return "0000";
  }
  std::string move;
  move += getFile(M.From) + 'a';
  move += getRank(M.From) + '1';
  move += getFile(M.To) + 'a';
  move += getRank(M.To) + '1';
  if (M.PromotedTo != NONE) {
    move += convertPieceToString(makePiece(BLACK, getType(M.PromotedTo)));
  }
  return move;
}

// adds castling availability [a] to castling mask [cm]
inline void addCastlingAvailability(CastlingMask& cm, char a) {
  if (a == '-') return;
  else if (a == 'k') cm |= 1;
  else if (a == 'q') cm |= 2;
  else if (a == 'K') cm |= 4;
  else if (a == 'Q') cm |= 8;
}

// deletes castling availability [a] to castling mask [cm]
inline void delCastlingAvailability(CastlingMask& cm, char a) {
  if (a == '-') return;
  else if (a == 'k') cm &= ~1;
  else if (a == 'q') cm &= ~2;
  else if (a == 'K') cm &= ~4;
  else if (a == 'Q') cm &= ~8;
}

inline bool getCastlingAvailability(CastlingMask& cm, char a) {
  if (a == 'k') return (cm & 1) != 0;
  else if (a == 'q') return (cm & 2) != 0;
  else if (a == 'K') return (cm & 4) != 0;
  else return (cm & 8) != 0;
}

// prints bitboard in human understandable
void outBb(Bitboard bb);

// adds length(x) - len spaces (' ') to x
std::string toLen(int64_t x, int len);

inline std::string score(Eval eval) {
  // engine mates
  if (eval > Mate - 1000) {
    return "mate  " + toLen((Mate - eval + 1) / 2, 5);
  // engine get mated
  } else if (eval < -Mate + 1000) {
    return "mate -" + toLen((eval + Mate + 1) / 2, 5);
  } else if (eval >= 0) {
    return "cp    " + toLen(eval, 5);
  } else {
    return "cp   -" + toLen(-eval, 5);
  }
}
