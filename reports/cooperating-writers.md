# Cooperating writer race check

## Target and source

Target: MONOLITH `main` (stable), release `v1.0.1`, commit `446e107efa466c47dbd5aa48333da1d7e879ede6`.

The harness forks two processes and calls the actual `Filesystem::writeFileWithProducerIfStampMatches` implementation and atomic publication sources against one temporary target. Each process receives the same initial file stamp and stages a distinct 256 KiB payload. Pipes hold both producer callbacks after they have written half their payload; the parent releases both only after receiving both ready markers.

## Properties checked

- Exactly one conditional writer returns `Written` and the other returns `Conflict` for the now-stale shared stamp.
- The visible file is exactly one complete writer payload, never a partial or mixed payload.
- No temporary staging entries remain beside the target after both operations complete.
- The temporary test directory is removed successfully.

## Bounds and assumptions

- Two cooperating processes, one target, one initial stamp, two fixed 262,144-byte payloads, and one forced overlapping staging interval.
- Uses POSIX `fork`, pipes, `poll`, advisory locking, and same-directory rename on the host filesystem. This does not cover non-cooperating writers, crash or power-loss behavior, injected syscall failures, arbitrary schedules, or general filesystem durability.
- Compiler: GCC/G++ `11.4.0`, C++23, ASan and UBSan. Leak detection was disabled for the successful run because enabling LeakSanitizer in this execution environment ended with its fatal `does not work under ptrace` runtime message. This result therefore does not assess leaks.
- This is an integration check, not a proof of all writer interleavings.

## Reproduction

From the audit repository root, with MONOLITH checked out at the target commit:

```sh
g++ -std=c++23 -Wall -Wextra -Wpedantic \
  -fsanitize=address,undefined -fno-omit-frame-pointer \
  -I../MONOLITH/src/fs -I../MONOLITH/src/detail \
  harnesses/filesystem/cooperating_writers.cpp \
  ../MONOLITH/src/fs/Filesystem.cpp \
  ../MONOLITH/src/detail/AtomicFile.cpp \
  ../MONOLITH/src/detail/AtomicTempOutput.cpp \
  -o /tmp/monolith-cooperating-writers
ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 \
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
  timeout 60s /tmp/monolith-cooperating-writers
```

## Result

The run exited 0 and printed `cooperating writer race passed: 2 processes, both staged before release, one Written, one Conflict, complete 262144-byte target`. ASan and UBSan emitted no report. Leak detection was not active. No MONOLITH finding was inferred.
