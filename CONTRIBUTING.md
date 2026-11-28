# Contributing

## Workflow

1. Fork the repository and create a branch from `main`.
2. Make your changes, adding tests in `tests/` where relevant.
3. Run the checks below locally.
4. Open a pull request describing what changed and why.

## Local checks

```sh
make WERROR=1
make test
make BUILD=sanitize test
make format-check
make tidy
```

`make format` rewrites sources using the project `.clang-format`.

## Code style

- C11, no compiler extensions.
- Formatting is enforced by `clang-format`; do not hand-format.
- Functions that are not part of a header's interface are `static`.
- Every `.c` module in `src/` (except `main.c`) is linked into the test binaries, so keep game logic out of `main.c` to make it testable. `main.c` is the only file that uses raylib, which keeps the tests headless.

## Adding tests

Create `tests/test_<name>.c` with its own `main`, include `test.h`, and use `EXPECT`, `EXPECT_EQ`, `RUN_TEST` and `TEST_REPORT`. The Makefile picks it up automatically.

## Commit messages

Use short, imperative subject lines, e.g. `Add level loader` or `Fix box push collision`.
