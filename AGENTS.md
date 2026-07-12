# Repository Guidelines

## Project Structure & Module Organization

`src/` contains portable C; `map.c` decodes maps, `raycast.c` traces walls and doors, `render.c` draws walls/sprites, `door.c` defines moving-door geometry, and `player.c` handles movement/collision. `tests/` contains C tests. `docs/specs/` contains SDD contracts. `data/` preserves the shareware ZIP and `.WL1` files; keep `data/README.md` current. `WOLFSRC/` is the untouched DOS reference. Root artifacts and `README.rst` must remain unchanged. CMake writes to ignored `build/`.

## Spec-Driven Workflow

Read the applicable file in `docs/specs/` before changing code. Add or revise its scope, non-goals, and acceptance criteria before implementation, then make the smallest change that satisfies them. Every functional change must update the milestone/status in the governing spec and the project state in `README.md`. Review this guide in the same change and update it whenever structure, commands, conventions, or contributor workflow differ; otherwise state that it was reviewed and remains accurate.

## Build, Test, and Development Commands

SDL2 development headers, a C compiler, and CMake are required.

```sh
./scripts/bootstrap.sh --install # Install dependencies on Debian/Ubuntu
./scripts/bootstrap.sh --check   # Verify dependencies without system changes
cmake -S . -B build       # Configure an out-of-tree build
cmake --build build       # Compile the Linux executable
./build/wolf3d --data data/shareware-v1.4 # Validate data and run
./build/wolf3d --check    # Run the headless SDL2 smoke check
ctest --test-dir build --output-on-failure # Run portable logic tests
```

On Windows, use Visual Studio 2022 x64 with SDL2 from vcpkg and configure CMake
with `-DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake`; build and
test with the `Release` configuration as documented in `README.md`.

Before submitting, also run `git diff --check` to catch whitespace errors. Do not commit anything under `build/`.

## Coding Style & Naming Conventions

Write portable C supported by the repository's CMake toolchain. Use four-space indentation, braces on their own line for functions, and short, explicit control flow. Follow `snake_case` for variables and functions, `UPPER_CASE` for constants, and SDL's established names for SDL types. Compilation enables `-Wall -Wextra -Wpedantic`; new code must build without warnings. Prefer C/SDL2 facilities already present over new dependencies or speculative abstractions.

## Testing Guidelines

CTest runs focused executables including `test_player`. Every change must build, pass CTest, and pass `./build/wolf3d --check`. Manually verify rendering, keyboard, and relative mouse input when affected. Add one small test for new non-trivial logic.

## Commit & Pull Request Guidelines

History is small and uses concise, sentence-style subjects. Prefer an imperative subject such as `Add SDL2 keyboard input`, keeping each commit focused. Pull requests should explain the Linux-port milestone, list verification commands, and call out changes to historical `WOLFSRC/` files. Include a screenshot for visible rendering changes and link the relevant issue when one exists.
