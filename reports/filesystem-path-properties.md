# Filesystem path helper bounded check

## Target and source

Target: MONOLITH `main` (stable), release `v1.0.1`, commit `446e107efa466c47dbd5aa48333da1d7e879ede6`.

The harness constructs `monolith::fs::Filesystem` and calls `normalize`, `join`, `isValidEntryName`, and `isSameOrDescendant` from the pinned `src/fs/Filesystem.cpp`.

## Properties checked

- Every enumerated input normalizes to an absolute path, normalization is idempotent, and every output component is non-empty and neither `.` nor `..`.
- Entry-name validity matches the implementation contract for empty, dot, dot-dot, ordinary, slash-containing, and embedded-NUL examples.
- Joining each ordinary tested entry name to a normalized parent returns a canonical path contained by that parent.
- Explicit same-path, child, root, and sibling-prefix cases confirm component-aware descendant behavior. Twenty additional parent/candidate pairs are checked against the component-boundary predicate.

## Bounds and assumptions

- Exhaustively enumerated 1,365 path strings of lengths 0 through 5 over `{a, b, ., /}`.
- The entry-name examples and 20 explicit containment pairs are additional finite cases.
- These are virtual-path string helpers only. The check does not resolve symlinks, inspect host filesystem containment, exercise writes, or cover arbitrary byte strings and longer paths.
- Compiler: GCC `11.4.0`, C++23, ASan and UBSan. Runtime timeout: 60 seconds.

## Reproduction

From the audit repository root, with MONOLITH checked out at the target commit:

```sh
g++ -std=c++23 -Wall -Wextra -Wpedantic \
  -fsanitize=address,undefined -fno-omit-frame-pointer \
  -I../MONOLITH/src/fs -I../MONOLITH/src/detail \
  harnesses/filesystem/path_properties.cpp \
  ../MONOLITH/src/fs/Filesystem.cpp \
  ../MONOLITH/src/detail/AtomicFile.cpp \
  ../MONOLITH/src/detail/AtomicTempOutput.cpp \
  -o /tmp/monolith-path-properties
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=halt_on_error=1 \
  timeout 60s /tmp/monolith-path-properties
```

## Result

The run exited 0 and printed `canonical paths checked: 1365; explicit containment pairs checked: 20`. No ASan or UBSan report occurred. This is a finite dynamic check, not an unbounded proof of path correctness or host containment.
