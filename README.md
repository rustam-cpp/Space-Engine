# Space

## About

Space is a UCI chess engine written in C++. It plays only standard chess. You can play with it on [lichess.org](https://lichess.org/@/SpaceEngine). 

## Details

### Move Generation

Space uses a custom bitboard move generator featuring:

- Precomputed attack masks for kings and knights
- Constant-time pawn attack generation
- Constant-time sliding piece attack generation
- Efficient in-check detection
- Early termination when only the existence of a legal move is required

The move generator is heavily based on bit manipulation and is designed to generate legal moves as efficiently as possible.

### Evaluation

Current versions of Space use a lightweight NNUE network with the following architecture:

```
772 -> 128 -> 1
```

Earlier versions of Space used a handcrafted evaluation including:

- Material evaluation
- Pawn structure evaluation
  - Passed pawns
  - Protected passed pawns
  - Isolated pawns
  - Doubled pawns
- Piece-square tables
- Smooth opening/endgame king evaluation
- King safety (pawn shield)
- Bishop pair bonus

### Search
- Quiescence search
- Alpha-beta pruning
- Iterative deepening
- Transposition table
- Principal Variation Search (PVS)
- Null Move Pruning (NMP)
- Late Move Reductions (LMR)
- Check extention
- Aspiration window

### Move ordering
- TT best move is always first
- MVV-LVA
- History heuristic
- Killer moves

### Time management
- Soft and hard bounds