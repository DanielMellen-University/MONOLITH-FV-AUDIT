# Terminal lexer bounded input check

## Target and source

Target: MONOLITH `main` (stable), release `v1.0.1`, commit `446e107efa466c47dbd5aa48333da1d7e879ede6`.

The harness calls `tokenizeCommandLine` and `completionContextAt` in the pinned `src/app/TerminalLexer.cpp` directly.

## Properties checked

For all enumerated command-line inputs, a non-empty lexer error implies that `args` is empty, so malformed quoting or a trailing escape does not return partial arguments. For every tested cursor, `completionContextAt` clamps the cursor to the line length and returns `replacementStart <= clampedCursor` and `prefix.size() <= clampedCursor`.

The run also exercises every parser/cursor branch induced by the finite alphabet under ASan and UBSan. Sanitizer silence is evidence only for these executions; it is not a general memory-safety or termination proof.

## Bounds and assumptions

- Exhaustively enumerated all 9,331 strings of length 0 through 5 over `{a, space, tab, backslash, single quote, double quote}`.
- For each string, checked cursors from 0 through `line.size() + 1`, inclusive: 63,452 cursor cases.
- No arbitrary-byte, longer-string, allocation-failure, or concurrent behavior is covered. The 30-second timeout bounds runtime; the completed run took under one second.
- Compiler: GCC `11.4.0`, C++17, `-Wall -Wextra -Wpedantic`, ASan and UBSan.

## Reproduction

From the audit repository root, with MONOLITH checked out at the target commit:

```sh
g++ -std=c++17 -Wall -Wextra -Wpedantic \
  -fsanitize=address,undefined -fno-omit-frame-pointer \
  -I../MONOLITH/src/app \
  harnesses/terminal-lexer/bounded_inputs.cpp \
  ../MONOLITH/src/app/TerminalLexer.cpp \
  -o /tmp/monolith-terminal-lexer-bounded
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=halt_on_error=1 \
  timeout 30s /tmp/monolith-terminal-lexer-bounded
```

## Result

The run exited 0 and printed `bounded lexer inputs passed: 9331; completion cursors passed: 63452`. No ASan or UBSan report occurred. This is a bounded dynamic check, not a proof for unbounded inputs.
