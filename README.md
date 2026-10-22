# MONOLITH Formal Verification Audit

This repository holds an independent assurance record for MONOLITH. The first run establishes scope, candidate properties, tool choices, and a reproducible baseline. It does not claim that MONOLITH has been formally verified.

The audit target is MONOLITH `main` (stable), commit `446e107efa466c47dbd5aa48333da1d7e879ede6`, release tag `v1.0.1`. When the baseline ran, `beta` pointed to the same commit, so its results carry over. Later audit re-pins follow `main` only. Audit artifacts, models, harnesses, scripts, and later results belong here. Build outputs belong in a scratch directory outside the MONOLITH checkout.

## Reproduce the baseline

From the MONOLITH repository root:

```sh
cmake -S . -B /tmp/monolith-fv-build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
cmake --build /tmp/monolith-fv-build --parallel 2
ctest --test-dir /tmp/monolith-fv-build --output-on-failure
```

The full ASan/UBSan baseline used:

```sh
cmake -S . -B /tmp/monolith-fv-sanitized -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON \
  -DCMAKE_CXX_FLAGS='-fsanitize=address,undefined -fno-omit-frame-pointer' \
  -DCMAKE_EXE_LINKER_FLAGS='-fsanitize=address,undefined'
cmake --build /tmp/monolith-fv-sanitized --parallel 2
BUILD_DIR=/tmp/monolith-fv-sanitized \
  CXXFLAGS='-g -fsanitize=address,undefined -fno-omit-frame-pointer' \
  ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
  UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
  ./scripts/run_headless_tests.sh
```

The sanitizer suite command compiles test executables under `BUILD_DIR`; the separate CMake build confirms the application target also builds with those instrumentation flags. See [baseline results](docs/04-baseline-results.md) for the observed results and limits.

## Audit documents

- [Scope and method](docs/00-scope-and-method.md)
- [Inventory and risk ranking](docs/01-inventory-and-risk.md)
- [Candidate properties](docs/02-properties.md)
- [Tooling assessment](docs/03-tooling.md)
- [Baseline results](docs/04-baseline-results.md)
- [Plan and open decisions](docs/PLAN.md)

## Current status

Run 1 is complete when its documentation is committed and pushed. No changes are made to the MONOLITH repository. Formal results will only be called proofs when a named tool discharges a stated property under recorded assumptions and bounds.
