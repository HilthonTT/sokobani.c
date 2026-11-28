# sokobani.c

[![CI](https://github.com/HilthonTT/sokobani.c/actions/workflows/ci.yml/badge.svg)](https://github.com/HilthonTT/sokobani.c/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)

A box-pushing puzzle game written in C11.

## Requirements

- A C11 compiler (GCC or Clang)
- GNU Make, `curl` and `tar`
- [raylib](https://www.raylib.com/) 5.5. If `pkg-config` finds an installed raylib it is used; otherwise `make` downloads and builds raylib into `build/deps/`. On Debian/Ubuntu that needs the X11 and OpenGL headers:

  ```sh
  sudo apt install libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libgl1-mesa-dev
  ```
- Optional: `clang-format` and `clang-tidy` for formatting and linting

## Building

```sh
make                  # debug build -> build/debug/sokobani
make BUILD=release    # optimized build -> build/release/sokobani
make BUILD=sanitize   # ASan + UBSan build
make run              # build and run
```

Use `CC=clang` or `CC=gcc` to pick a compiler and `WERROR=1` to treat warnings as errors. Run `make clean` after switching compilers.

## Playing

```sh
make run                     # start at level 1
./build/debug/sokobani 4     # start at level 4
```

Push every crate onto a goal marker. A crate turns green when it sits on a goal. The window is resizable and the board scales to fit.

| Key                      | Action                 |
| ------------------------ | ---------------------- |
| arrows / `WASD` / `HJKL` | move                   |
| `U` / `Z` / Backspace    | undo                   |
| `R`                      | restart level          |
| `N` / `P`                | next / previous level  |
| Enter / Space            | continue after a level |
| `?` / F1                 | show or hide controls  |
| Esc                      | quit                   |

## Credits

The interface font is [PixAntiqua](https://www.dafont.com/pixantiqua.font) by Gerhard Großmann (freeware), taken from the raylib examples. It is in `assets/fonts/` and is embedded in the executable at build time.

## Testing

```sh
make test
make BUILD=sanitize test
```

## Development

```sh
make format         # apply clang-format
make format-check   # verify formatting
make tidy           # run clang-tidy
make help           # list targets and variables
```

## Installing

```sh
make BUILD=release
sudo make BUILD=release install           # installs to /usr/local/bin
make BUILD=release PREFIX=$HOME/.local install
```

## Project layout

```
src/        game sources
assets/     fonts embedded into the executable
tests/      unit tests and the minimal test harness
build/      build output (ignored)
.github/    CI workflow and issue/PR templates
```

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md).

## License

MIT, see [LICENSE](LICENSE).
