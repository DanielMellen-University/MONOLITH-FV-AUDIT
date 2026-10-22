# Assurance summary, first model and harness pass

## Target and result

Target: MONOLITH `main` (stable), release `v1.0.1`, commit `446e107efa466c47dbd5aa48333da1d7e879ede6`.

No confirmed MONOLITH defect was identified in this pass. No MONOLITH source file was modified. The findings register remains empty. This is not a whole-program formal verification result.

## Properties verified by formal tools

- TLC `2026.10.20.183001` exhaustively checked the window lifecycle model's `TypeOK` and `FocusInvariant` over its complete 276-distinct-state graph. Bounds: two unique window identities and callback depth at most two. These are properties of the abstract TLA+ model, not a proof of C++ refinement. See [window lifecycle model](../reports/window-lifecycle.md).
- TLC exhaustively checked `TypeOK`, `WorkspaceShape`, `NoPartialPublication`, `PublishedOnlyAfterCompleteSync`, and `ReclaimSafety` over the complete 34-distinct-state atomic workspace model. Bounds: one writer, one cleaner, and two content chunks. POSIX rename and cooperating advisory-lock assumptions apply; no power-loss durability or multiple-writer contention result is established. See [atomic workspace model](../reports/atomic-workspace.md).
- CBMC `6.9.0` verified three assertions in the isolated smoke harness for all six assumed index values `-1..4`, with count fixed at four and no loops. This verifies only the toy helper, not MONOLITH source. See [tool smoke tests](tool-smoke-tests.md#cbmc-c-assertion-harness).

## Properties checked dynamically or statically

- The baseline CMake Debug build and all 34 registered tests passed. The full baseline ASan/UBSan suite passed without a report. These checks cover only executed test paths. The warning-heavy build passed with 269 diagnostics, mostly conversions and generated dependency warnings. Clang Static Analyzer reported no diagnostics on six selected translation units. Details and commands are in [baseline results](04-baseline-results.md).
- The actual `TerminalLexer.cpp` passed 774 finite completion round trips and bounded checks over 9,331 inputs and 63,452 cursor values under ASan/UBSan. The lengths, six-character alphabet, and properties are in the linked reports in [RESULTS.md](RESULTS.md). These are not proofs for longer or arbitrary-byte input.
- The actual `DrawingRaster.cpp` passed small `.modr` round trips for nine dimension pairs, 225 two-chunk split cases, and 16 malformed-length rejection cases under ASan/UBSan. Maximum dimensions and arbitrary malformed streams are not covered.
- The actual filesystem path methods passed normalization checks over 1,365 strings, six entry-name examples, and 20 explicit containment pairs under ASan/UBSan.
- Five serial conditional-write cases passed under ASan/UBSan: absent-target write, matching-stamp replacement, stale-stamp conflict, producer failure preservation, and expected-absent conflict.
- The existing focused window lifecycle test binary passed in both normal and sanitizer builds. The run exercised callback reentrancy, deferred close, surviving windows, and focus handoff. These regressions do not prove memory safety generally.

Every command, tool version, finite bound, assumption, and result is indexed in [RESULTS.md](RESULTS.md).

## Tool limitation

CBMC `6.9.0` exited during parsing of GCC 11 standard library headers when invoked on the actual terminal lexer translation unit. No MONOLITH property was analyzed. The failed parse is documented as a tool compatibility limit, not a source finding. The source-level lexer and codec checks above are dynamic finite checks only.

## Scope limits and next work

The atomic workspace model includes one writer and one cleanup scanner. It does not model two distinct writers contending for publication, and no behavior is claimed for non-cooperating host writers. See the open decision in [PLAN.md](PLAN.md) before treating cooperating multi-writer serialization as modeled.

The next audit work should resolve that model scope, then extend checked coverage to more filesystem error/interleaving cases and decide whether to evaluate a different bounded checker for STL-dependent units. Findings remain report-first; fixes require a confirmed finding and its evidence in [FINDINGS.md](FINDINGS.md).
