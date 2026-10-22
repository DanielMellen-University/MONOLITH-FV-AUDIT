# Tool smoke tests

These runs only establish that a locally installed tool launches and accepts a minimal invocation. They do not check MONOLITH properties.

## TLC CLI launch

- Tool: TLA+ tools `v1.8.0`, TLC version string `2026.10.20.183001`.
- Runtime: OpenJDK `11.0.32.1`.
- Install command: `./scripts/install_tlc.sh`.
- Download source: `https://github.com/tlaplus/tlaplus/releases/download/v1.8.0/tla2tools.jar`.
- Expected official SHA-1: `ed814d7792505bbd141096ae66a0c4c5d878d951`.
- Observed SHA-1: `ed814d7792505bbd141096ae66a0c4c5d878d951`.
- Observed SHA-256: `5bd7b64db99ae4c6c35f290de5e809c0ee41af43f8699cf696099ffb6452b1e2`.
- CLI launch command: `java -jar tools/tla2tools.jar -help`.
- Checked: JVM and TLC CLI start and print usage/version output.
- Exact property: no MONOLITH correctness property was checked. This is an installation/launch check only.
- Bounds: none.
- Assumptions: Java runtime is usable; network and GitHub release contents are trusted subject to the recorded checksum.
- Result: TLC reports `Version 2026.10.20.183001`. The help invocation exits 1 after displaying usage, as TLC treats it as a command-line usage request. The next section records a successful model-checking smoke run.
- Installation location: ignored `tools/`, not committed. No sudo used.

## TLC finite-model smoke

See [`specs/smoke/TLCSmoke.tla`](../specs/smoke/TLCSmoke.tla) and its config. This is a toy installation check, not a model of MONOLITH.

- Tool: TLC `2026.10.20.183001` from TLA+ tools `v1.8.0`; OpenJDK `11.0.32.1`.
- Reproduction command: `java -XX:+UseParallelGC -jar tools/tla2tools.jar -workers 1 -metadir /tmp/monolith-fv-tlc-smoke-rerun -config specs/smoke/TLCSmoke.cfg specs/smoke/TLCSmoke.tla`.
- Checked property: `TypeOK == n \in 0..2` is invariant under `Spec`.
- Bounds: `n` has values 0, 1, and 2; `Next` increments until 2, then stutters. TLC exhaustively explored the finite reachable graph: 4 states generated, 3 distinct states, depth 3.
- Assumptions: the TLA+ model is the entire object checked; it makes no claim about MONOLITH or its C++ implementation. TLC's state fingerprints are accepted for this smoke check.
- Result: TLC reported `Model checking completed. No error has been found.` and exited 0. This invariant is verified for the stated finite model only.
- Time box: under one second, no explicit timeout needed.
