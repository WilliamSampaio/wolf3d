# Repository Guidelines

## Project Structure & Module Organization

`src/` contains the new portable C implementation; `src/main.c` currently owns the SDL2 window, framebuffer, and event loop. `docs/specs/` contains the SDD contracts that govern implementation. `WOLFSRC/` is the untouched Borland C++ 3.x/DOS source release and serves as the behavioral reference. Root-level distribution artifacts and `README.rst` are historical and must remain unchanged. CMake writes generated files to the ignored `build/` directory.

## Spec-Driven Workflow

Read the applicable file in `docs/specs/` before changing code. Add or revise its scope, non-goals, and acceptance criteria before implementation, then make the smallest change that satisfies them. Every functional change must update the milestone/status in the governing spec and the project state in `README.md`. Review this guide in the same change and update it whenever structure, commands, conventions, or contributor workflow differ; otherwise state that it was reviewed and remains accurate.

## Build, Test, and Development Commands

SDL2 development headers, a C compiler, and CMake are required.

```sh
cmake -S . -B build       # Configure an out-of-tree build
cmake --build build       # Compile the Linux executable
./build/wolf3d            # Run the SDL2 framebuffer demo; Esc exits
./build/wolf3d --check    # Run the headless SDL2 smoke check
```

Before submitting, also run `git diff --check` to catch whitespace errors. Do not commit anything under `build/`.

## Coding Style & Naming Conventions

Write portable C supported by the repository's CMake toolchain. Use four-space indentation, braces on their own line for functions, and short, explicit control flow. Follow `snake_case` for variables and functions, `UPPER_CASE` for constants, and SDL's established names for SDL types. Compilation enables `-Wall -Wextra -Wpedantic`; new code must build without warnings. Prefer C/SDL2 facilities already present over new dependencies or speculative abstractions.

## Testing Guidelines

There is no test framework or coverage target yet. Every change must build and pass `./build/wolf3d --check`. For rendering or input changes, manually run the game and describe what was verified. Add a small focused test only when introducing non-trivial portable logic; use names such as `test_map_loader.c`.

## Commit & Pull Request Guidelines

History is small and uses concise, sentence-style subjects. Prefer an imperative subject such as `Add SDL2 keyboard input`, keeping each commit focused. Pull requests should explain the Linux-port milestone, list verification commands, and call out changes to historical `WOLFSRC/` files. Include a screenshot for visible rendering changes and link the relevant issue when one exists.
