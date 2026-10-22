# Terminal completion escaping check

## Target and source

Target: MONOLITH `main` (stable), release `v1.0.1`, commit `446e107efa466c47dbd5aa48333da1d7e879ede6`.

The harness calls `monolith::app::escapeCompletion` and `monolith::app::tokenizeCommandLine` from the pinned `src/app/TerminalLexer.cpp`. It does not copy or reimplement either function.

## Property checked

For every non-empty value of length 1 through 3 over the alphabet `{a, space, tab, backslash, single quote, double quote}`, and each quote mode `{unquoted, single quoted, double quoted}`, wrapping the result of `escapeCompletion(value, quote)` in that quote context and passing it to `tokenizeCommandLine` produces exactly one argument equal to `value` and no error.

The empty string is excluded because an empty unquoted completion does not create a token in the command lexer. This check does not claim round-trip behavior for every insertion context, cursor position, arbitrary byte, or longer input.

## Bounds and assumptions

- Enumerated all 6 + 36 + 216 = 258 non-empty values, three modes each, for 774 cases.
- Standard C++17 implementation of the source functions, compiled with GCC 11.4.0 and AddressSanitizer/UndefinedBehaviorSanitizer.
- Assertion failures abort the run. This finite harness is an exhaustive check of the stated finite domain, not a proof for unbounded strings.

## Reproduction

From the audit repository root, with MONOLITH checked out at the target commit:

```sh
g++ -std=c++17 -Wall -Wextra -Wpedantic \
  -fsanitize=address,undefined -fno-omit-frame-pointer \
  -I../MONOLITH/src/app \
  harnesses/terminal-lexer/completion_roundtrip.cpp \
  ../MONOLITH/src/app/TerminalLexer.cpp \
  -o /tmp/monolith-terminal-lexer-roundtrip
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=halt_on_error=1 \
  /tmp/monolith-terminal-lexer-roundtrip
```

## Result

The run passed all 774 cases and emitted `completion round trips passed: 774`. The sanitizer run emitted no ASan or UBSan report. One initial exploratory case included the empty value in unquoted mode and failed because the lexer correctly returns zero arguments for empty input; the stated property was narrowed to non-empty token values, and that precondition is explicit above.
