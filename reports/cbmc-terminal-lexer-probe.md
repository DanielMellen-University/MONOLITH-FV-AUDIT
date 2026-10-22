# CBMC compatibility probe for the terminal lexer

## Purpose

Test whether selected CBMC `6.9.0` can parse and analyze the pinned `TerminalLexer.cpp` directly, before investing in a bounded harness for that code.

## Target and command

MONOLITH `main` (stable), release `v1.0.1`, commit `446e107efa466c47dbd5aa48333da1d7e879ede6`.

From the audit repository root:

```sh
timeout 180s tools/cbmc-6.9.0-root/usr/bin/cbmc \
  --cpp11 --function tokenizeCommandLine -I../MONOLITH/src/app \
  ../MONOLITH/src/app/TerminalLexer.cpp --show-properties
```

Tool version: CBMC `6.9.0`. Time box: 180 seconds; command failed during parsing in under one second.

## Result and limits

CBMC exited 6 with parsing errors in the GCC 11 standard library headers `type_traits`, `bits/move.h`, and `bits/stl_pair.h`, before analyzing MONOLITH code. No property was checked and no finding follows from this failure. This establishes a compatibility limit for this CBMC build and the host standard library configuration, not a defect in `TerminalLexer.cpp`.

Direct bounded proof of this STL-dependent function will require a supported front end/configuration or a small, explicitly justified extraction. The independent exhaustive C++ harness remains a finite-domain check only.
