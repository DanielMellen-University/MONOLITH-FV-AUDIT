# Final assurance summary

## Target and conclusion

Target: MONOLITH `main` (stable), release `v1.0.1`, commit `446e107efa466c47dbd5aa48333da1d7e879ede6`.

Phases 0-5 of the audit plan are complete for the bounds and assumptions recorded in the linked reports. No confirmed MONOLITH defect was identified. No MONOLITH source file was modified. This is a bounded assurance exercise, not whole-program formal verification.

## Model-checked results

- TLC `2026.10.20.183001` exhaustively checked `TypeOK` and `FocusInvariant` over the window lifecycle model's 276 distinct states. Bounds: two unique window identities and callback depth at most two. The model abstracts SDL, app/controller destruction, close vetoes, and other implementation details; this is not a C++ refinement proof. See [window lifecycle model](../reports/window-lifecycle.md).
- TLC exhaustively checked five invariants over 34 distinct states in the atomic workspace model. Bounds: one writer, one cleanup scanner, and two content chunks. POSIX rename and cooperating advisory-lock semantics are assumptions. See [atomic workspace model](../reports/atomic-workspace.md).
- TLC exhaustively checked four invariants over 28 distinct states (41 generated, depth 13) in the two-writer publication model. Both writer identities must finish staging and syncing before lock contention begins. This model excludes cleanup, stamp conflicts, syscall failures, crashes, non-cooperating writers, and fairness; OS lock behavior is assumed, not verified. See [cooperating writers model](../reports/atomic-writers.md).
- CBMC `6.9.0` verified three assertions in its isolated smoke harness for all six assumed index values `-1..4`, with count fixed at four and no loops. The actual terminal lexer translation unit could not be parsed because CBMC rejected GCC 11 standard-library headers; no MONOLITH property was checked by CBMC. See [tool smoke tests](tool-smoke-tests.md#cbmc-c-assertion-harness) and [lexer parse probe](../reports/cbmc-terminal-lexer-probe.md).

## Dynamically and statically checked results

- The baseline Debug build and all 34 registered tests passed. The ASan/UBSan baseline suite passed; the warning-heavy build completed with 269 diagnostics, mostly conversions and generated dependency warnings; Clang Static Analyzer reported no diagnostics on six selected translation units. These checks cover only their executed paths. See [baseline results](04-baseline-results.md).
- The actual `TerminalLexer.cpp` passed finite completion and malformed-input checks, then a 10-second libFuzzer run checked 709,371 inputs with maximum length 512. The actual `DrawingRaster.cpp` passed finite `.modr` round-trip/split checks, then a 10-second libFuzzer run checked 2,619,917 inputs with maximum length 512. ASan and UBSan reported no issue during the successful fuzz runs. The fuzz runs are sampling, not exhaustive proof. See the reports indexed in [RESULTS.md](RESULTS.md).
- Actual filesystem path helpers passed finite normalization/containment checks. Five serial conditional-write cases passed. A coordinated two-process test staged both 262,144-byte payloads before release and observed exactly one `Written`, one stale-stamp `Conflict`, a complete visible payload, and no staging residue. These checks do not establish behavior for arbitrary schedules, host symlinks, non-cooperating writers, syscall failures, crashes, or power loss. See the filesystem reports in [RESULTS.md](RESULTS.md).
- The window model was manually mapped to `WindowManager` methods and callback paths. Focused lifecycle and focus regressions passed in normal and sanitized binaries during the audit. The tests cover named reentrancy, deferred-close, survivor, and focus-handoff traces; they do not prove general C++ memory safety or refinement. See [window lifecycle refinement review](../reports/window-lifecycle-refinement.md).

## Tool limits and residual risk

- LeakSanitizer terminated with its fatal ptrace-related runtime message in an enabled run. Leak detection was disabled for the successful fuzz campaigns and cooperating-writer integration run. Those runs provide no leak assessment, while ASan and UBSan remained enabled.
- One high-volume seed-pinned lexer fuzz retry exited 139 before producing libFuzzer output; two 1,000-run retries then exited 0. Its cause is unresolved, so the successful 10-second run remains the reported campaign and the startup failure is retained as a tooling/runtime limitation.
- CBMC's actual-source lexer parse failure is a tooling compatibility limit, not a source defect. Clang 14 fuzz targets required explicit GCC 11 standard-library include and linker paths.
- TLA+ results establish only properties of the specified finite models. They do not prove C++ refinement, OS lock/rename semantics, durability, fairness, or liveness.
- Fuzzing and bounded harnesses cover only their recorded finite inputs, sizes, and durations. Static analyzer silence is not proof.

The findings register remains empty after final triage. Exact properties, commands, versions, bounds, and results are indexed in [RESULTS.md](RESULTS.md); the completed phase status is in [PLAN.md](PLAN.md).
