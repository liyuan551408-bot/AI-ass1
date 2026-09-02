# Massey Cloud Build and Run

This folder is configured for the Massey cloud Linux environment.

## Build

```bash
make
```

The Linux build creates:

```text
search.out
```

## Run

Run all batch experiments:

```bash
./search.out batch_run all
```

Run one algorithm:

```bash
./search.out single_run astar_explist_manhattan 120483765 123456780
./search.out single_run astar_explist_misplacedtiles 120483765 123456780
./search.out single_run uc_explist 120483765 123456780
```

Run animation:

```bash
./search.out animate_run astar_explist_manhattan 120483765 123456780
```

## Clean

```bash
make clean
```

## Notes

The Windows build uses `search.exe`, `mingw32-make`, and `gdi32`.
The Massey cloud build uses `search.out`, `make`, `g++`, `SDL_bgi`, and `SDL2`.
