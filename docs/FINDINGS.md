# Findings

This is the running register of confirmed findings. Add an ID only after validating the evidence against the pinned MONOLITH source. Tool warnings and model counterexamples remain unconfirmed until reviewed.

| ID | Component | Severity | Evidence | Status |
|---|---|---|---|---|
| None | No confirmed defects found through the completed Phase 5 triage (2026-10-22). | - | Baseline, finite TLA+ models, bounded and fuzzed parser/codec/path checks, serial and coordinated multi-process conditional-write checks, and focused window lifecycle regressions produced no confirmed MONOLITH defect. CBMC's direct lexer parse failure and LeakSanitizer's runtime failure are tool/environment limitations, not source findings. | No findings |

When a finding is confirmed, include the source pin, exact reproduction, affected invariant, impact, and status. Before any fix, write the evidence here. If fixed, include the MONOLITH commit hash and regression test; mark the finding fixed only after both repositories contain the linked evidence.
