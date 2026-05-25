#include "move.h"
#include "converts.h"
#include "types.h"

// builds struct Move from position and givven start and
// finish squares and piece after promotion (if exists)
Move makeMove(const Position& pos, Square From, Square To, Piece PromotedTo) {
  Move move;
  move.From = From;
  move.To = To;
  move.Moved = pos.getPiece(From);
  move.Captured = pos.getPiece(To);
  move.PromotedTo = PromotedTo;
  // check is castling
  if (getType(move.Moved) == KING && abs(getFile(From) - getFile(To)) > 1)
    move.Castling = true;
  // check is en passant
  if (getType(move.Moved) == PAWN && move.Captured == NONE && abs(getFile(From) - getFile(To)) == 1)
    move.EnPassant = true;
  // okay, now values from Position pos
  move.Mask = pos.Castlings;
  move.EnPassantSquare = pos.EnPassantSquare;
  move.HalfmoveClock = pos.HalfmoveClock;
  return move;
}

// plays move in givven position
void doMove(Position& pos, const Move& move, rt* RT) {

  // 1. move Pieces
  if (move.EnPassant) {
    // move moved piece
    pos.resetPiece(move.From, move.Moved);
    pos.setPiece(move.To, move.Moved);
    // delete eaten pawn
    Square EatenPawnSquare = move.To + (move.From < move.To ? -8 : 8);
    pos.resetPiece(EatenPawnSquare, swapColor(move.Moved));
  } else if (move.Castling) {
    Square To = move.From + (move.From < move.To ? 2 : -2);
    // calculating current rook square
    Square RookSquare = move.From + (move.From < To ? 3 : -4);
    // calculating new rook square
    Square newRookSquare = To + (move.From < To ? -1 : 1);
    // calculating rook
    Piece Rook = makePiece(getColor(move.Moved), ROOK);
    // move rook
    pos.resetPiece(RookSquare, Rook);
    pos.setPiece(newRookSquare, Rook);
    // move king
    pos.resetPiece(move.From, move.Moved);
    pos.setPiece(To, move.Moved);
  } else {
    // move out moved piece from move.From
    pos.resetPiece(move.From, move.Moved);
    // move out captured piece from move.To
    pos.resetPiece(move.To, move.Captured);
    if (move.PromotedTo != NONE)
      // set promoted piece to move.To
      pos.setPiece(move.To, move.PromotedTo);
    else
      // set moved piece to move.To
      pos.setPiece(move.To, move.Moved);
  }
  
  pos.ZobristHash ^= pos.Castlings;
  // 2. change castling availability if rook moved
  if (getType(move.Moved) == ROOK) {
    // square 0 is a1
    // square 7 is h1
    // square 56 is a8
    // square 63 is h8
    if (getColor(move.Moved) == WHITE) {
      if (move.From == 0) delCastlingAvailability(pos.Castlings, 'Q');
      else if (move.From == 7) delCastlingAvailability(pos.Castlings, 'K');
    } else {
      if (move.From == 56) delCastlingAvailability(pos.Castlings, 'q');
      else if (move.From == 63) delCastlingAvailability(pos.Castlings, 'k');
    }
  // change castling availability if king moved
  } else if (getType(move.Moved) == KING) {
    if (getColor(move.Moved) == BLACK) {
      delCastlingAvailability(pos.Castlings, 'k');
      delCastlingAvailability(pos.Castlings, 'q');
    } else {
      delCastlingAvailability(pos.Castlings, 'K');
      delCastlingAvailability(pos.Castlings, 'Q');
    }
  }
  // change castling availability if the move is castling
  if (move.Castling) {
    if (getColor(move.Moved) == WHITE) {
      delCastlingAvailability(pos.Castlings, 'Q');
      delCastlingAvailability(pos.Castlings, 'K');
    } else {
      delCastlingAvailability(pos.Castlings, 'q');
      delCastlingAvailability(pos.Castlings, 'k');
    }
  }
  // change castling availability if the rook was captured
  if (getType(move.Captured) == ROOK) {
    if (getColor(move.Moved) == BLACK) {
      if (move.To == 0) delCastlingAvailability(pos.Castlings, 'Q');
      else if (move.To == 7) delCastlingAvailability(pos.Castlings, 'K');
    } else {
      if (move.To == 56) delCastlingAvailability(pos.Castlings, 'q');
      else if (move.To == 63) delCastlingAvailability(pos.Castlings, 'k');
    }
  }
  pos.ZobristHash ^= pos.Castlings;

  pos.ZobristHash ^= pos.EnPassantSquare << 12;
  pos.EnPassantSquare = -1;
  // 3. calculate en passant target square
  if (getType(move.Moved) == PAWN) {
    if (abs(getRank(move.From) - getRank(move.To)) == 2) {
      // this square is arithmetic average
      pos.EnPassantSquare = (move.From + move.To) / 2;
    }
  }
  pos.ZobristHash ^= pos.EnPassantSquare << 12;

  // 4. update other variables
  // 4.1 update fullmove clock
  if (!pos.WhiteToMove) pos.FullmoveClock++;

  pos.ZobristHash ^= pos.WhiteToMove << 4;
  pos.WhiteToMove = !pos.WhiteToMove;
  pos.ZobristHash ^= pos.WhiteToMove << 4;

  // 4.2 update halfmove clock
  if (getType(move.Moved) == PAWN || move.Captured != NONE)
    pos.HalfmoveClock = 0;
  else
    pos.HalfmoveClock++;

  RT->add(pos.ZobristHash);

}

// undoes move in givven position
void undoMove(Position& pos, const Move& move, rt* RT) {

  RT->del();

  // it's copy + paste doMove, but all is opposite
  if (move.EnPassant) {
    pos.resetPiece(move.To, move.Moved);
    pos.setPiece(move.From, move.Moved);
    Square EatenPawnSquare = move.To + (move.From < move.To ? -8 : 8);
    pos.setPiece(EatenPawnSquare, swapColor(move.Moved));
  } else if (move.Castling) {
    Square To = move.From + (move.From < move.To ? 2 : -2);
    Square RookSquare = move.From + (move.From < To ? 3 : -4);
    Square newRookSquare = To + (move.From < To ? -1 : 1);
    Piece Rook = makePiece(getColor(move.Moved), ROOK);
    pos.resetPiece(newRookSquare, Rook);
    pos.setPiece(RookSquare, Rook);
    pos.resetPiece(To, move.Moved);
    pos.setPiece(move.From, move.Moved);
  } else {
    if (move.PromotedTo != NONE)
      pos.resetPiece(move.To, move.PromotedTo);
    else
      pos.resetPiece(move.To, move.Moved);
    pos.setPiece(move.To, move.Captured);
    pos.setPiece(move.From, move.Moved);
  }
  // reset other values
  pos.EnPassantSquare = move.EnPassantSquare;

  pos.Castlings = move.Mask;

  pos.HalfmoveClock = move.HalfmoveClock;

  if (pos.WhiteToMove) pos.FullmoveClock--;

  pos.WhiteToMove = !pos.WhiteToMove;

  pos.ZobristHash = RT->stack.back();

}

uint64_t hashMove(const Move& move) {
  return           (move.From << 19) ^
                   (move.To << 26) ^
         ((uint64_t)move.Moved << 33) ^
         ((uint64_t)move.Captured << 40) ^
         ((uint64_t)move.PromotedTo << 47) ^
         ((uint64_t)move.EnPassant << 54);
}