# Baseline results

## Target and environment

- Target repository branch: `beta`
- Target commit: `446e107efa466c47dbd5aa48333da1d7e879ede6`
- Target checkout status: clean before and after the audit commands
- Host: Linux x86_64, kernel `6.8.0-138-generic`
- Compiler: GCC 11.4.0; Clang 14.0.0 also present
- CMake 3.22.1, SDL2 2.0.20, SDL2_ttf 2.0.18, Python 3.10.12, Java 11
- Scratch build directories: `/tmp/monolith-fv-build`, `/tmp/monolith-fv-sanitized`, `/tmp/monolith-fv-warnings`

## Build and full headless suite

Commands:

```sh
cmake -S /home/daniel/Documents/GitHub/MONOLITH -B /tmp/monolith-fv-build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
cmake --build /tmp/monolith-fv-build --parallel 2
ctest --test-dir /tmp/monolith-fv-build --output-on-failure
```

Result: configure and full `monolith` target build succeeded. CMake generated/decompressed build inputs and fetched the pinned `stb_image.h` into the scratch build. Registered CTest `monolith_headless` passed. Its test log reported all five static integration checks and all 34 headless test programs passed, with `ALL HEADLESS TESTS PASSED`; CTest recorded 106.52 seconds. These tests cover selected behaviors, not all possible inputs or states.

## ASan/UBSan suite

Commands:

```sh
cmake -S /home/daniel/Documents/GitHub/MONOLITH -B /tmp/monolith-fv-sanitized -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON \
  -DCMAKE_CXX_FLAGS='-fsanitize=address,undefined -fno-omit-frame-pointer' \
  -DCMAKE_EXE_LINKER_FLAGS='-fsanitize=address,undefined'
cmake --build /tmp/monolith-fv-sanitized --parallel 2
BUILD_DIR=/tmp/monolith-fv-sanitized \
  CXXFLAGS='-g -fsanitize=address,undefined -fno-omit-frame-pointer' \
  ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
  UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
  /home/daniel/Documents/GitHub/MONOLITH/scripts/run_headless_tests.sh
```

Result: sanitized application build succeeded. The headless runner completed all five static checks and 34 tests with `ALL HEADLESS TESTS PASSED`; no ASan, leak, or UBSan report occurred. This is evidence only for exercised paths, enabled sanitizer checks, and this environment. It does not establish absence of memory errors or undefined behavior.

## Warning-heavy compiler build

Command:

```sh
cmake -S /home/daniel/Documents/GitHub/MONOLITH -B /tmp/monolith-fv-warnings -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS='-Wconversion -Wsign-conversion -Wshadow -Wformat=2 -Wnull-dereference'
cmake --build /tmp/monolith-fv-warnings --parallel 2
```

Result: build succeeded. The log contained 269 warning diagnostics. 188 were emitted from generated `stb_image.h`, three from generated `main_body_a.inc`, and the rest were largely conversion/sign-conversion warnings in project translation units or generated project includes. This warning count is translation-unit diagnostic output, not 269 distinct defects. There was one shadow warning. The baseline did not suppress generated or third-party warnings; later triage should separate them before evaluating project warning debt.

The ordinary project flags `-Wall -Wextra -Wpedantic` produced no warnings in the full Debug build.

## Static analysis

First attempt with `clang++ --analyze` failed because Clang 14 did not locate GCC's C++ standard library headers automatically. Retried with explicit host header paths:

```sh
clang++ --analyze -std=c++2b \
  -isystem /usr/include/c++/11 \
  -isystem /usr/include/x86_64-linux-gnu/c++/11 \
  -Isrc $(pkg-config --cflags sdl2 SDL2_ttf) \
  src/fs/Filesystem.cpp src/detail/AtomicFile.cpp src/detail/AtomicTempOutput.cpp \
  src/app/TerminalLexer.cpp src/app/DrawingRaster.cpp src/settings/DesktopSettings.cpp \
  -Xanalyzer -analyzer-output=text
```

Result: exit status 0 and no diagnostics for those six translation units. This was a focused analyzer pass, not whole-program analysis; only the host headers/libraries and each translation unit's analyzed paths were considered.

## Tool inventory

`cbmc`, `esbmc`, `frama-c`, `z3`, `tlc`, `clang-tidy`, `cppcheck`, and `scan-build` were not available on `PATH`. Java 11 is present. No tool was installed and no system-wide changes were made.

## Checkout integrity

`git status --porcelain` in MONOLITH was empty after baseline work. All configure/build/test products and warning/analyzer logs were under `/tmp`. The audit repository contains the written reports and will contain subsequent specs, harnesses, and results.
