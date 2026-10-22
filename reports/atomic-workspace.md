# Atomic workspace publication and cleanup model

## What is checked

[`specs/filesystem/AtomicWorkspace.tla`](../specs/filesystem/AtomicWorkspace.tla) abstracts the write lifecycle in `detail::writeAtomically`, `createAtomicTempDirectory`, `publishAtomicTempFile`, `AtomicTempCleanup::~AtomicTempCleanup`, and the locked cleanup path in `tryScavengeAtomicTempWorkspace` / `tryReclaimReadyAtomicTempDirectory` in `src/detail/AtomicFile.{hpp,cpp}`.

The model interleaves one cooperating writer with one cleanup scanner. It models the parent-directory lock, a writer lease, a valid or foreign workspace, bounded content progress, stream completion, content sync, atomic publication, abort/crash, and stale-workspace scanning.

## Properties

- `TypeOK`: every modeled field remains in its finite declared domain.
- `WorkspaceShape`: absent workspaces have no owner/lease; valid and foreign workspace identities match their modeled owner class; a live lease implies a valid owned workspace with a ready marker.
- `NoPartialPublication`: target visibility is always the old or new complete value; the model has no externally visible partial-content state. A new value is only visible after the publication transition.
- `PublishedOnlyAfterCompleteSync`: when publication has occurred, the stream had reached two chunks and the content sync step completed.
- `ReclaimSafety`: the cleanup path never records reclamation of an invalid-owner or live-lease workspace.

## Bounds and assumptions

- One target and one writer protocol are modeled, with one cleanup scanner interleaving at action boundaries. The parent lock has one owner at a time. Multiple independent writer identities are not modeled.
- Content is bounded to two chunks: 0 means no content written, 1 means partial, 2 means complete. Publication requires 2 and `ContentSynced`.
- The initial state is either a new write, a stale owned V4 workspace with a ready marker and no live lease, or a foreign/unknown workspace.
- Cleanup only reclaims the modeled valid owned ready workspace when the lease is free and entries are known. Legacy workspace formats, incomplete pre-ready V4 setup recovery, candidate token collisions, and directory traversal budgeting are outside this first model.
- `flock` is modeled as mutually exclusive advisory locking honored by cooperating MONOLITH writers/cleaners. Non-cooperating host writers are explicitly excluded. Filesystem check/use races, symlink retargets, process crashes at individual syscall boundaries, and actual POSIX syscall failures are abstracted to the `Abort` and `CrashWriter` actions.
- POSIX same-parent rename publication is treated as one atomic state transition. This model checks visibility atomicity, not power-loss durability or directory-fsync guarantees. A stamp mismatch can abort before publication but the external stamp is assumed stable while the parent lock is held.
- No liveness, starvation, fairness, or throughput property is checked. Deadlock detection is disabled because finite terminal phases are intentional; safety invariants are still checked over the reachable state graph.
- This is an abstract model, not a refinement proof of the C++ implementation.

## Reproduction

From the audit repository root, with TLC `2026.10.20.183001` / TLA+ tools `v1.8.0` and OpenJDK `11.0.32.1`:

```sh
timeout 180s java -XX:+UseParallelGC -jar tools/tla2tools.jar \
  -workers 1 \
  -metadir /tmp/monolith-fv-atomic-workspace-run1 \
  -config specs/filesystem/AtomicWorkspace.cfg \
  specs/filesystem/AtomicWorkspace.tla
```

The time box is 180 seconds. TLC metadata and traces are written under `/tmp`.

## Result

TLC `2026.10.20.183001` with OpenJDK `11.0.32.1` exited 0 in under one second. It generated 76 states, found 34 distinct reachable states, exhausted the queue, and reached depth 10. It reported no invariant violation for `TypeOK`, `WorkspaceShape`, `NoPartialPublication`, `PublishedOnlyAfterCompleteSync`, or `ReclaimSafety` in this finite model. The reported fingerprint collision probability estimate was `7.7E-17`.

This is a model check of the abstract transition system, not a proof that the C++ implementation refines it. In particular, no POSIX syscall behavior or crash durability is verified.
