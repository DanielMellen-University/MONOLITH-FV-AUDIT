# `.modr` codec bounded round-trip check

## Target and source

Target: MONOLITH `main` (stable), release `v1.0.1`, commit `446e107efa466c47dbd5aa48333da1d7e879ede6`.

The harness calls `encodeModr`, `decodeModr`, and `ModrStreamDecoder` in the pinned `src/app/DrawingRaster.cpp` directly.

## Properties checked

- For each canvas dimension pair from 1x1 through 3x3, encoded length equals 12 header bytes plus three bytes per pixel. Decoding returns the original dimensions and RGB channels; decoded alpha is 255 for every pixel.
- For every split offset, including 0 and end-of-stream, feeding the encoded bytes as two chunks to `ModrStreamDecoder` and finishing returns the same decoded image.
- Every strict prefix of a valid 1x1 encoding and the same encoding with one extra byte is rejected by `decodeModr`. On these failures the caller's width, height, and output vector remain unchanged.

## Bounds and assumptions

- Nine dimension pairs, each with deterministic pixel data. A two-chunk split is tested at each byte offset; arbitrary multi-chunk partitions are not covered.
- Fifteen truncation lengths and one extended length are checked for the 1x1 valid encoding.
- Dimensions, payloads, and chunk boundaries are small. This does not check max-dimension arithmetic, allocation limits, arbitrary bytes, or all malformed headers.
- Compiler: GCC `11.4.0`, C++17, ASan and UBSan. Runtime timeout: 60 seconds.

## Reproduction

From the audit repository root, with MONOLITH checked out at the target commit:

```sh
g++ -std=c++17 -Wall -Wextra -Wpedantic \
  -fsanitize=address,undefined -fno-omit-frame-pointer \
  -I../MONOLITH/src/app \
  harnesses/drawing-raster/modr_roundtrip.cpp \
  ../MONOLITH/src/app/DrawingRaster.cpp \
  -o /tmp/monolith-modr-roundtrip
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=halt_on_error=1 \
  timeout 60s /tmp/monolith-modr-roundtrip
```

## Result

The run exited 0 and printed `small .modr canvases passed: 9; chunk split cases passed: 225; malformed length cases passed: 16`. No ASan or UBSan report occurred. This is a finite dynamic check, not a proof for general images or malformed streams.
