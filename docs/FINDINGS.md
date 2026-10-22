# Findings

This is the running register of confirmed findings. Add an ID only after validating the evidence against the pinned MONOLITH source. Tool warnings and model counterexamples remain unconfirmed until reviewed.

| ID | Component | Severity | Evidence | Status |
|---|---|---|---|---|
| None | No confirmed defects recorded yet. | - | The baseline, finite TLA+ models, bounded parser/codec/path checks, conditional-write integration cases, and focused window lifecycle regressions produced no confirmed MONOLITH defect. CBMC's direct lexer parse failure is a tool compatibility limit, not a source finding. | No findings |

When a finding is confirmed, include the source pin, exact reproduction, affected invariant, impact, and status. Before any fix, write the evidence here. If fixed, include the MONOLITH commit hash and regression test; mark the finding fixed only after both repositories contain the linked evidence.
