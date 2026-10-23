# Cooperating atomic writers model

## What is checked

[`specs/filesystem/AtomicWriters.tla`](../specs/filesystem/AtomicWriters.tla) supplements the single-writer-plus-cleaner model in [atomic workspace](atomic-workspace.md). It models two distinct writers staging complete contents independently and then contending for the same exclusive publication lock before replacing one target.

The first lock acquisition is disabled until both writer contents are fully staged and synced. This deliberately reaches the contention state: both writers are publication-ready while only one can own the lock. Publication appends the winning writer identity to a commit-order sequence and atomically changes the visible target token to that identity.

## Properties

- `TypeOK`: both progress counters, the sync set, lock owner, visible target, and commit sequence stay in their finite domains; each writer appears at most once in the sequence.
- `LockDiscipline`: an owned publication lock belongs to a synced writer that has not already committed.
- `CommittedContentComplete`: every writer in the publication history had written both bounded chunks and synced its content before publication.
- `PublicationOrder`: the visible target equals the last writer in the serialized commit history, or remains the old value before any publication.

Together, the transitions and invariants check that each writer publishes only while owning the shared lock and that all complete commits have a total order. They do not establish filesystem or C++ refinement.

## Bounds and assumptions

- Exactly two cooperating writer identities, one target, one shared exclusive publication lock, and at most one commit per writer.
- Each writer stages exactly two abstract chunks. Both are fully written and synced before either may acquire the publication lock.
- The visible target is abstracted as `None` (old value) or a writer identity (that writer's complete value). There is no partial-value state.
- This extension does not include a cleanup scanner, conditional stamp conflicts, expected-absent rename, workspace ownership/leases, crashes, syscall failures, non-cooperating host writers, or fairness/liveness. Cleanup safety and those lifecycle cases remain covered only to the bounds in [atomic workspace](atomic-workspace.md).
- Lock acquisition is modeled as one indivisible exclusive owner assignment; POSIX `flock` and same-directory rename semantics are assumptions, not verified OS behavior.

## Reproduction

From the audit repository root with TLC `2026.10.20.183001`, TLA+ tools `v1.8.0`, and OpenJDK `11.0.32.1`:

```sh
timeout 180s java -XX:+UseParallelGC -jar tools/tla2tools.jar \
  -workers 1 \
  -metadir /tmp/monolith-fv-atomic-writers-final \
  -config specs/filesystem/AtomicWriters.cfg \
  specs/filesystem/AtomicWriters.tla
```

The time box is 180 seconds. TLC metadata is written under `/tmp`.

## Result

TLC exited 0 with `Model checking completed. No error has been found.` It checked all four invariants over 41 generated states, 28 distinct states, and depth 13; no states remained on the queue. The fingerprint collision estimate was `2.0E-17`. This verifies the finite abstract model only. An earlier check exposed an incorrect model invariant about lock ownership immediately after publication; the invariant was corrected before this recorded run. That was a model correction, not a MONOLITH finding.
