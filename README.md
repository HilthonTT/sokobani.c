# sokobani.c

[![CI](https://github.com/HilthonTT/sokobani.c/actions/workflows/ci.yml/badge.svg)](https://github.com/HilthonTT/sokobani.c/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)

A box-pushing puzzle game written in C11.

## Requirements

- A C11 compiler (GCC or Clang)
- GNU Make
- Optional: `clang-format` and `clang-tidy` for formatting and linting

## Building

```sh
make                  # debug build -> build/debug/sokobani
make BUILD=release    # optimized build -> build/release/sokobani
make BUILD=sanitize   # ASan + UBSan build
make run              # build and run
```

Use `CC=clang` or `CC=gcc` to pick a compiler and `WERROR=1` to treat warnings as errors. Run `make clean` after switching compilers.

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
tests/      unit tests and the minimal test harness
build/      build output (ignored)
.github/    CI workflow and issue/PR templates
```

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md).

## License

MIT, see [LICENSE](LICENSE).
