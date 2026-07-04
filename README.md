# Space

## About

Space is a UCI chess engine written in C++. It plays only standard chess. You can play with it on [lichess.org](https://lichess.org/@/SpaceEngine). 

## Details

### Move Generation
I write a pretty strange move generation:

- precalculated masks for kings and knights
- the O(1) calculations for pawns
- the O(1) calculations for bishops, rooks and queens, but they're heavy
- the O(1) function to check if there is check on the board.
- if we need to check if there are legal moves in the position, we can generate only 1 move (if it's exist)

So, it's a pretty fast move generation function, because I use many bit manipulation functions, but it can be improved

### Evaluation
- NNUE

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