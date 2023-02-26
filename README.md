# eternity2-solver
My playground for Eternity II Puzzle solver

I took the naive approach of placing one piece after another in defined order. When there is no option for the next position, fall back to the previous one and choose another piece. After `FALLBACKS` number of fallbacks, it prints the maximum reached position and prints the current board. Moreover, if the maximum position is above 190, it prints the board with every fallback peak.

## Optimizations

1. The biggest one is using the precomputed arrays of fitting pieces. With that, I got to 0.06 s for 1,000,000 fallbacks.
