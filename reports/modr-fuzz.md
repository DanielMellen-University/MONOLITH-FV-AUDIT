# MODR codec fuzz check

## Target and source

Target: MONOLITH `main` (stable), release `v1.0.1`, commit `446e107efa466c47dbd5aa48333da1d7e879ede6`.

The libFuzzer target sends arbitrary bytes through both `decodeModr` and `ModrStreamDecoder`, feeding the stream decoder in variable chunks of 1-23 bytes. It requires whole-buffer and streaming results to agree; failure must preserve caller outputs; successful results must have bounded positive dimensions, the expected RGBA length and opaque alpha bytes, and encode back to the exact canonical input.

## Bounds and assumptions

- One bounded campaign, empty initial corpus, input maximum 512 bytes, 10 seconds of libFuzzer time.
- Clang/Clang++ `14.0.0`, compiler-rt libFuzzer, GCC 11 standard-library headers and `libstdc++` supplied explicitly for this host. ASan and UBSan were enabled. LeakSanitizer was disabled because the lexer target's enabled run showed its fatal ptrace-related runtime failure; leaks are not assessed.
- Fuzzing samples inputs and schedules; it is not exhaustive and proves no property for all valid or malformed files. A time-bounded run may execute a different number of inputs on another machine.

## Reproduction

From the audit repository root:

```sh
clang++ -std=c++2b -O1 -g -fsanitize=fuzzer,address,undefined \
  -fno-omit-frame-pointer \
  -isystem /usr/include/c++/11 \
  -isystem /usr/include/x86_64-linux-gnu/c++/11 \
  -L/usr/lib/gcc/x86_64-linux-gnu/11 \
  -I../MONOLITH/src/app \
  harnesses/drawing-raster/fuzz.cpp \
  ../MONOLITH/src/app/DrawingRaster.cpp \
  -o /tmp/monolith-modr-fuzz
mkdir -p /tmp/monolith-modr-artifacts
ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 \
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
  timeout 20s /tmp/monolith-modr-fuzz \
  -max_len=512 -max_total_time=10 -print_final_stats=1 -verbosity=0 \
  -artifact_prefix=/tmp/monolith-modr-artifacts/
```

Clang 14 spells the draft C++23 mode `-std=c++2b`.

## Result

The run exited 0 after 10 seconds with 2,619,917 executed inputs and 28 new corpus units; no crash, assertion failure, ASan report, or UBSan report occurred. Peak RSS reported by libFuzzer was 704 MiB. This is one bounded fuzz campaign, not an exhaustive codec result.
