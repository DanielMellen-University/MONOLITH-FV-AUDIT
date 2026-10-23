# Terminal lexer fuzz check

## Target and source

Target: MONOLITH `main` (stable), release `v1.0.1`, commit `446e107efa466c47dbd5aa48333da1d7e879ede6`.

The libFuzzer target calls the actual `tokenizeCommandLine` and `completionContextAt` from `TerminalLexer.cpp` with arbitrary byte strings. For each input it checks that a parse error does not return partial arguments, and probes completion at the start, midpoint, end, and one position beyond the end. It checks that replacement/prefix offsets stay within the clamped cursor and that quote state is one of the supported values.

## Bounds and assumptions

- One bounded campaign, empty initial corpus, input maximum 512 bytes, 10 seconds of libFuzzer time.
- Clang/Clang++ `14.0.0`, compiler-rt libFuzzer, GCC 11 standard-library headers and `libstdc++` supplied explicitly for this host. ASan and UBSan were enabled. LeakSanitizer was disabled after an enabled run ended in its fatal `does not work under ptrace` runtime message; leaks are not assessed.
- Fuzzing samples inputs and schedules; it is not exhaustive and establishes no property for all inputs or execution orders. A time-bounded run may execute a different number of inputs on another machine.

## Reproduction

From the audit repository root:

```sh
clang++ -std=c++17 -O1 -g -fsanitize=fuzzer,address,undefined \
  -fno-omit-frame-pointer \
  -isystem /usr/include/c++/11 \
  -isystem /usr/include/x86_64-linux-gnu/c++/11 \
  -L/usr/lib/gcc/x86_64-linux-gnu/11 \
  -I../MONOLITH/src/app \
  harnesses/terminal-lexer/fuzz.cpp \
  ../MONOLITH/src/app/TerminalLexer.cpp \
  -o /tmp/monolith-lexer-fuzz
mkdir -p /tmp/monolith-lexer-artifacts
ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 \
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
  timeout 20s /tmp/monolith-lexer-fuzz \
  -max_len=512 -max_total_time=10 -print_final_stats=1 -verbosity=0 \
  -artifact_prefix=/tmp/monolith-lexer-artifacts/
```

## Result

The run exited 0 after 10 seconds with 709,371 executed inputs and 823 new corpus units; no crash, assertion failure, ASan report, or UBSan report occurred. The initial enabled-LeakSanitizer run executed 822,216 inputs before the runtime itself terminated with the ptrace-related fatal message; that exit was not a finding and is not counted as a successful run. One later seed-pinned 700,000-run retry exited 139 before producing libFuzzer output; two 1,000-run retries, with and without that seed, exited 0. The cause of that intermittent startup failure is unresolved, and it is recorded as a tool/runtime limitation rather than a source finding.
