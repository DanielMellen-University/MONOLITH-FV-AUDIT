# Plan

This is a staged audit. Phase 0 established the foundation and baseline. Decisions below were made by Daniel on 2026-10-22.

## Proposed phases

Cost estimates are rough Codex effort for the stated artifact, excluding tool installation delays and human review. XS/S/M/L indicate artifact size, not confidence.

| Phase | Deliverable and exit condition | Size | Rough Codex cost |
|---|---|---:|---:|
| 0. Foundation and baseline | Scope, risk map, candidate properties, tooling choices, reproducible build/test/sanitizer/static-analysis baseline, and decisions. This run. | S, 7 docs | 8k-14k tokens; 1 run |
| 1. Compact models | TLA+ model for window ownership/focus/deferred close and a separate atomic-workspace publication/cleanup lifecycle model. TLC checks finite bounds and records state counts/counterexamples. | M, 2 specs + configs + reports | 18k-32k tokens; 1-2 runs |
| 2. Parser and codec harnesses | Pure C++ harnesses for `TerminalLexer` and `DrawingRaster`/`.modr`; bounded checker if selected, plus exhaustive/property tests or fuzz harnesses. Record the alphabet/input/loop bounds and completeness limits. | M, 2-4 harnesses | 22k-40k tokens; 1-2 runs |
| 3. Filesystem path and write contracts | Extract or harness normalization, containment, descendant guards, and conditional-write boundary logic. Model filesystem operations and lock assumptions; prove/check only tractable helper contracts and tie them to existing integration tests. | L, 3-6 artifacts | 30k-55k tokens; 2-3 runs |
| 4. Window-manager refinement | Map the abstract lifecycle model to methods and callback paths in `WindowManager`; add invariant/property tests for small event traces, then check reentrancy transitions against the model. | L, 3-5 artifacts | 30k-50k tokens; 2-3 runs |
| 5. Triage and report | Deduplicate counterexamples, classify confirmed issues versus assumptions/tool limitations, produce final assurance report, and prepare fixes only under the selected decision. | M, report + issue records | 18k-35k tokens; 1-2 runs |

## DECIDED (2026-10-22)

### 1. Tools and installation policy: A

Install Clang Static Analyzer, ASan/UBSan, and TLC locally into the audit repo's gitignored `tools/` directory; use a local JRE if needed. CBMC `6.9.0` is selected after a local C++ harness smoke check. Keep all tools local and do not use sudo.

### 2. Components for proof and lighter checking: A

Use bounded code-level proofs for pure parser, codec, and path helpers. Use finite TLA+ models for filesystem publication/cleanup and window lifecycle. Use static analysis, fuzzing, sanitizers, and tests for the rest.

### 3. Handling findings in MONOLITH: C

Write the findings report in this audit repo first. Then fix confirmed findings directly on MONOLITH `main` (stable). A finding must be confirmed and recorded in `FINDINGS.md` with evidence before a fix begins.

### 4. Model scope and assumptions: A

Model in-process callbacks/reentrancy and cooperating atomic writers. Explicitly exclude non-cooperating host writers and state the POSIX publication assumptions.

## Working rules

- The target is MONOLITH `main` (stable), currently release `v1.0.1` at `446e107efa466c47dbd5aa48333da1d7e879ede6`. After any fix lands, re-pin to the new `main` commit and record it. Leave `beta` alone.
- Work through phases in order. Commit and push each finished piece to audit `main` promptly. Do not batch completed work for a final push.
- For every harness, model, or check, document the checked behavior, exact property, bounds, assumptions, reproducible command, tool version, and result.
- Maintain [FINDINGS.md](FINDINGS.md) with finding ID, component, severity, evidence, and status; maintain [RESULTS.md](RESULTS.md) as the index of proved or checked work.
- Be strict about proof language. Say “verified” only when a tool proved the property and state the bounds. Dynamic and static checks are not proofs.
- Fix only confirmed findings after writing them up with evidence. Make a small focused MONOLITH `main` commit with a regression test, run the full normal and sanitizer suites, pull with rebase before pushing, and link the fix commit from `FINDINGS.md`. Do not force-push or include unrelated changes.
- If a fix is large or changes user-visible behavior, do not implement it. Add an OPEN DECISIONS entry with 2 to 4 options labeled A/B/C and one recommendation, then continue independent work.

## Progress after the first model and harness pass (2026-10-22)

- Phase 0 complete: baseline and audit foundation are committed.
- Phase 1 complete for the recorded bounds: TLC exhausted the window lifecycle and atomic workspace models. The filesystem model has one writer and one cleaner, so it does not establish behavior under contention between multiple distinct writers.
- Phase 2 first pass complete: actual terminal lexer and `.modr` source were checked with finite ASan/UBSan harnesses. CBMC 6.9.0 could not parse the lexer translation unit's GCC 11 standard library headers, so these results are bounded dynamic checks, not code-level proofs.
- Phase 3 first pass complete: path helper checks and serial conditional-write cases passed. The atomic filesystem model states POSIX publication assumptions and excludes non-cooperating host writers. Broader operation/error interleavings remain follow-up work.
- Phase 4 first pass complete: the abstract window model was traced to implementation methods and existing focused lifecycle tests passed in normal and sanitizer binaries. This is not a C++ refinement proof.
- Phase 5 initial triage complete: no confirmed MONOLITH finding. See [FINDINGS.md](FINDINGS.md), [RESULTS.md](RESULTS.md), and the [assurance summary](05-assurance-summary.md).

## OPEN DECISIONS

### 1. Multiple cooperating writers in the filesystem model

The current finite model has one writer and one cleanup scanner. The parent lock is modeled with exclusive ownership, but two distinct writer lifecycles are not interleaved.

A. Extend the model to two writer identities with separate workspaces contending for the shared publication lock. Recommended. This provides direct finite coverage of cooperating writer serialization, within a bounded two-writer model.

B. Keep the current single-writer model and treat inter-writer serialization as a source-review and integration-test obligation.

C. Leave multi-writer interleavings outside the formal model and state that limit in the final assurance summary.

Recommendation: A. The model should exercise both writers reaching publication readiness and contending for the lock before treating cooperating-writer serialization as covered.
