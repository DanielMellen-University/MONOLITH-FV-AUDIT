# Plan

This is a staged audit. The next run should follow the accepted open decisions below; run 1 only establishes the foundation and baseline.

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

## OPEN DECISIONS

### 1. Tool and installation policy

- **A.** Use Clang Static Analyzer, ASan/UBSan, and TLC installed into ignored audit-local `tools/`; smoke-test one local CBMC or ESBMC release before selecting it. Do not use sudo. **Recommendation.** This is sufficient to start useful modeling and bounded helper checks without changing the host.
- **B.** Approve sudo/system package installation for ESBMC or CBMC, `cppcheck`, and supporting dependencies; retain TLC local or packaged.
- **C.** Use only currently installed tools and defer all model checking/bounded checking until a later environment is available.

### 2. Components for real proofs versus lighter checking

- **A.** Reserve code-level bounded proofs for pure parser/codec/path helpers; use finite TLA+ models for filesystem publication/cleanup and window lifecycle; use static analysis, fuzzing, sanitizers, and tests for the rest. **Recommendation.** It aligns tool strengths to tractable boundaries and avoids claims about external SDL/POSIX behavior.
- **B.** Attempt bounded verification directly on the complete `Filesystem.cpp` and `WindowManager.cpp`, accepting harness abstraction and substantial modeling work.
- **C.** Treat the audit as dynamic/static checking only and make no formal proof claims or model-checking artifacts.

### 3. Handling later findings in MONOLITH

- **A.** Report findings here first; after Daniel reviews them, implement approved fixes on a new topic branch based on `main`, with separate commits/PRs in MONOLITH. **Recommendation.** This keeps the audit evidence independent and protects `beta` while preserving a path to fixes.
- **B.** Report findings only in this audit repository; do not change MONOLITH.
- **C.** Fix confirmed findings directly on `beta` after an audit report is written.

### 4. Model scope and assumptions

- **A.** Model in-process callbacks/reentrancy and cooperating atomic writers; explicitly exclude non-cooperating host writers and model POSIX publication assumptions. **Recommendation.** This makes obligations checkable without hiding the documented external race.
- **B.** Include adversarial host filesystem mutation between every check and publication step, even where the implementation documents that such writers do not honor its lock.
- **C.** Treat all filesystem operations as atomic and ignore external writers in the model.
