#include "board.h"
#include "converts.h"
#include <iostream>

void Position::resetPiece(Square S, Piece P) {
  if (P == NONE) return;
  ZobristHash ^= Zobrist[P][S];
  board[S] = NONE;
  if (getColor(P) == WHITE)
    WhitePieces &= ~(1ULL << S);
  else
    BlackPieces &= ~(1ULL << S);
  pieces[P] &= ~(1ULL << S);

  delFeature(acc, nnue, featureIndex(P, S));
}

void Position::setPiece(Square S, Piece P) {
  Piece p = getPiece(S);
  if (p != NONE) {
    ZobristHash ^= Zobrist[p][S];
    resetPiece(S, p);
  }
  board[S] = P;
  if (P == NONE) return;
  ZobristHash ^= Zobrist[P][S];
  if (getColor(P) == WHITE)
    WhitePieces |= (1ULL << S);
  else
    BlackPieces |= (1ULL << S);
  pieces[P] |= (1ULL << S);

  addFeature(acc, nnue, featureIndex(P, S));
}

Piece Position::getPiece(Square S) const {
  return board[S];
}

void Position::convertFromFen(std::string FEN, rt* RT) {

  // reseting all values
  std::fill(pieces, pieces + 32, 0);
  std::fill(board, board + 64, NONE);
  WhitePieces = 0;
  BlackPieces = 0;
  Castlings = 0;

  // Pointer to symbols in FEN string
  int i = 0;

  // 1. The actual position (piece placement)
  int rank = 7;
  int file = 0;
  // we will read all symbols up to the first space
  while (FEN[i] != ' ') {
    // symbol '/' means that we move to the next rank
    if (FEN[i] == '/') {
      rank--; file = 0;
    }
    // number [x] means that we skip x squares
    else if (FEN[i] >= '0' && FEN[i] <= '9') {
      // a fast way to convert digit (char) to int
      file += FEN[i] - '0';
    }
    // piece notation means that we set this piece to current square
    else {
      setPiece(makeSquare(rank, file), convertStringToPiece(FEN[i]));
      file++;
    }
    // move to the next symbol
    i++;
  }

  i++;
  
  // 2. Side to move
  if (FEN[i] == 'w')
    WhiteToMove = true;
  else
    WhiteToMove = false;

  // move to the next character and space skip in one line
  i += 2;

  // 3. Castling availability
  while (FEN[i] != ' ') {
    addCastlingAvailability(Castlings, FEN[i]);
    i++;
  }

  i++;

  // 4. En passant target square
  if (FEN[i] == '-') {
    // No en passant available, skip space and '-'
    EnPassantSquare = -1;
    i += 2;
  } else {
    // .substr(pos, len) returns next len characters after ith (inclusive)
    EnPassantSquare = convertStringToSquare(FEN.substr(i, 2));

    // skip space and square
    i += 3;
  }

  // 5. Halfmove clock (count of moves after last pawn move or capture)
  std::string number;
  while (FEN[i] != ' ') {
    number += FEN[i];
    i++;
  }

  // stoi converts std::string to int (int32_t)
  // there is also stoll and it converts std::string to long long (int64_t)
  HalfmoveClock = stoi(number);

  i++;

  number.clear();
  // 6. Fullmove clock (count of moves by 2 sides). Increases after each black move
  while (i < (int)FEN.size() && FEN[i] != ' ') {
    number += FEN[i];
    i++;
  }
  
  FullmoveClock = stoi(number);

  // generate zobrist hash for a new position
  ZobristHash = getZobristHash();

  RT->clear();
  RT->add(ZobristHash);

  initAccumulator(acc, nnue, *this);

}

// just a beautiful position output fuction for debug
std::ostream& operator<<(std::ostream& out, Position& P) {
  out << "  +---+---+---+---+---+---+---+---+\n";
  int rank = 7;
  int file = 0;
  while (rank >= 0) {
    out << rank + 1 <<  " |";
    while (file < 8) {
      out << ' ' << convertPieceToString(P.getPiece(makeSquare(rank, file))) << " |";
      file++;
    }
    out << '\n';
    out << "  +---+---+---+---+---+---+---+---+\n";
    rank--;
    file = 0;
  }
  out << "    a   b   c   d   e   f   g   h" << std::endl;
  return out;
}

uint64_t Position::getZobristHash() const {
  uint64_t ZH = 0;
  for (Square S = 0; S < 64; S++) {
    Piece P = getPiece(S);
    if (P == NONE) continue;
    ZH ^= Zobrist[P][S];
  }
  ZH ^= Castlings;
  ZH ^= WhiteToMove << 4;
  ZH ^= EnPassantSquare << 12;
  return ZH;
}

bool Position::isFiftyMoveDraw() const {
  return HalfmoveClock >= 100;
}

bool Position::isRepetitionDraw(rt* RT) {
  return RT->isDraw(ZobristHash, HalfmoveClock);
}