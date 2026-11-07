# 8-Puzzle Search Algorithms

A C++ assignment project that solves the 8-puzzle and compares search strategies. The program works with a 3 × 3 board, where `0` is the empty space, and reports a solution path and search statistics for several starting states.

## What it demonstrates

- Uniform-cost search and A* search.
- A* with misplaced-tiles and Manhattan-distance heuristics.
- Priority queues, duplicate-state handling and deterministic tie breaking.
- Comparison of path length, expanded states, queue size and running time.

The algorithm implementations are in [algorithm.cpp](A1_windows%20%28VSCode%29/A1_windows%20%28VSCode%29/algorithm.cpp). The program also includes a graphical board and predefined experiments in [main.cpp](A1_windows%20%28VSCode%29/A1_windows%20%28VSCode%29/main.cpp).

## Build and run

The repository contains two Windows project copies. Start with `A1_windows (VSCode)/A1_windows (VSCode)/`. Its makefile builds `search.exe` with a C++11 compiler and the Windows graphics library supplied with the project.

From that directory, with MinGW and `mingw32-make` available:

```powershell
mingw32-make
.\search.exe
```

The makefile also has macOS/Linux branches that require SDL BGI and SDL2. Those environments are not documented as tested here.

## Reading the results

The program uses five predefined starting states and goal state `123456780`. It prints each algorithm's solution path, path length, state expansions, maximum queue length and running time. Compare the number of states explored by uniform-cost search with A* using each heuristic; a shorter runtime in one run alone is not a reliable benchmark.

## Repository context

This is a university assignment repository. It includes supplied starter code, assignment instructions and experiment spreadsheets. The search logic is easiest to inspect in `algorithm.cpp`; the included project copies and supporting files are retained for the original coursework environment.
