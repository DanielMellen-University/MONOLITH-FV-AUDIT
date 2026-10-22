# Conditional atomic write integration check

## Target and source

Target: MONOLITH `main` (stable), release `v1.0.1`, commit `446e107efa466c47dbd5aa48333da1d7e879ede6`.

The harness runs `Filesystem::writeFileWithProducerIfStampMatches`, `fileStamp`, `readFile`, and the actual atomic publication implementation in `Filesystem.cpp`, `AtomicFile.cpp`, and `AtomicTempOutput.cpp` against a fresh temporary directory.

## Properties checked

Five serial cases check these outcomes:

1. An absent target with expected stamp `nullopt` is written and contains the producer bytes.
2. A matching existing `FileStamp` permits replacement and the complete replacement bytes are visible.
3. A stale prior stamp returns `Conflict` and leaves the current bytes unchanged.
4. A producer that writes partial staged content and returns false returns `Failed` and leaves the current target bytes unchanged.
5. Expected absence against an existing target returns `Conflict` and does not replace its bytes.

## Bounds and assumptions

- One process, one target, five serial operations, small byte strings. No callback reentrancy, concurrent writer, external writer, symlink, crash, power loss, or injected syscall failure is tested here.
- The test relies on the host POSIX filesystem behavior used by the implementation, including same-directory atomic rename and advisory locks among cooperating writers. It is an integration check, not a proof of durability or race freedom.
- Compiler: GCC `11.4.0`, C++23, ASan and UBSan. Runtime timeout: 120 seconds.

## Reproduction

From the audit repository root, with MONOLITH checked out at the target commit:

```sh
g++ -std=c++23 -Wall -Wextra -Wpedantic \
  -fsanitize=address,undefined -fno-omit-frame-pointer \
  -I../MONOLITH/src/fs -I../MONOLITH/src/detail \
  harnesses/filesystem/conditional_write.cpp \
  ../MONOLITH/src/fs/Filesystem.cpp \
  ../MONOLITH/src/detail/AtomicFile.cpp \
  ../MONOLITH/src/detail/AtomicTempOutput.cpp \
  -o /tmp/monolith-conditional-write
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=halt_on_error=1 \
  timeout 120s /tmp/monolith-conditional-write
```

## Result

The run exited 0 and printed `conditional write cases passed: 5`. No ASan or UBSan report occurred. No MONOLITH finding was inferred from these cases.
