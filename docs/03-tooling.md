# Tooling assessment

## Machine checked on 2026-10-22

Available: GCC/G++ 11.4.0, Clang/Clang++ 14.0.0, CMake 3.22.1, Ninja, Make, Python 3.10.12, Java 11, pkg-config, SDL2 2.0.20, SDL2_ttf 2.0.18. Clang Static Analyzer is available through `clang++ --analyze`; Clang's C++ standard-library include search did not find GCC headers automatically, so the successful baseline command supplied `/usr/include/c++/11` and `/usr/include/x86_64-linux-gnu/c++/11` explicitly.

Not found on `PATH`: `cbmc`, `esbmc`, `frama-c`, `z3`, `tlc`, `clang-tidy`, `cppcheck`, `scan-build`, and `scan-build-14`. Python has `pytest`; the `z3`, `hypothesis`, `lit`, and `mesonbuild` modules are absent. Java is installed, but the TLA+ tools JAR is not known to be installed.

No software was installed for this run. No `sudo` was used.

## Options

| Tool or method | Fit here | Installation and limitations |
|---|---|---|
| Clang Static Analyzer | Useful now for path-sensitive bug finding on C++ translation units. A six-TU focused pass completed without diagnostics. | Already part of installed Clang 14. For repeatable whole-project analysis, install/run `scan-build` or CodeChecker and generate a compilation database. Analyzer silence is not proof. LLVM documents path-sensitive interprocedural analysis for C/C++ ([docs](https://clang.llvm.org/docs/ClangStaticAnalyzer.html)). |
| ASan/UBSan | Useful now; the full headless test suite passed instrumented. Catches only executed memory/UB errors. | Already supported by GCC 11 runtimes here. The exact flags and inputs are in `README.md`. ASan detects memory errors and UBSan instruments selected UB checks ([ASan](https://clang.llvm.org/docs/AddressSanitizer.html), [UBSan](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html)). No sudo needed. |
| CBMC | Selected bounded checker for small extracted helpers. A C++ assertion harness passed in 6.9.0. A direct parse probe of `TerminalLexer.cpp` failed in GCC 11 standard library headers before reaching MONOLITH code, so source compatibility must be established per unit. | Installed locally by extracting the Ubuntu 22.04 `.deb` under ignored `tools/`; no package install and no sudo. STL, POSIX filesystem behavior, SDL, templates, and C++23 features can limit scope. State loop/input bounds and unwinding checks. Official project describes CBMC as a C/C++ bounded model checker ([README](https://github.com/diffblue/cbmc/blob/develop/README.md)). See [tool smoke tests](tool-smoke-tests.md#cbmc-c-assertion-harness) and [lexer probe](../reports/cbmc-terminal-lexer-probe.md). |
| ESBMC | Strong candidate for extracted C++ helpers: memory, arithmetic, pointer, and assertion checks with bounded or induction strategies. C++ frontend supports C++20 and selected C++23 features. | Not installed. Official Ubuntu PPA suggests a system package route that requires sudo; a standalone release or container could avoid system install but must be checked for this host's ABI/dependencies. C++23 and STL coverage are partial. A fixed unwind is a bounded result; check unwinding assertions ([C++ docs](https://esbmc.github.io/docs/c-cpp/), [algorithm docs](https://esbmc.github.io/docs/theory/verification-algorithms/)). |
| TLA+ with TLC | Best fit for window lifecycle/focus/deferred-close and atomic cleanup/publication state models. Java 11 is already present. | Installed locally as TLA+ tools `v1.8.0` in ignored `tools/`; CLI smoke test passed. No sudo. Java 11 satisfies the documented runtime requirement ([TLC setup](https://docs.tlapl.us/using%3Atlc%3Astart)). TLC checks a finite model, not the C++ implementation. See [tool smoke tests](tool-smoke-tests.md). |
| Alloy Analyzer | Alternative for compact relational state and bounded instance checks; Alloy 6 also supports temporal model checking. | Not installed. A Java distribution can be kept in ignored `tools/`; some temporal workflows require external NuSMV/nuXmv, so TLC may be simpler here ([Alloy documentation](https://alloytools.org/documentation.html)). |
| libFuzzer + property tests | Good supporting method for lexer, `.modr`, session/settings parsers, and pure helper boundaries. Existing tests already provide useful seeds/oracles. | Clang 14 exists, but verify fuzzer runtime support with a one-file smoke build before committing. libFuzzer is bundled with LLVM compiler-rt; no sudo if supported by installed runtime. Fuzzing is input sampling, not proof. |
| Frama-C / ACSL | Poor direct fit because MONOLITH is C++ and uses C++ ownership, STL, exceptions, and SDL. | Frama-C targets C; translating selected C-like helpers to C or maintaining a second implementation would introduce a refinement burden. Frama-C's official docs cover Frama-C and ACSL ([docs](https://www.frama-c.com/html/documentation.html)). |
| Cppcheck / clang-tidy | Useful conventional static checks, but lower priority than current analyzer, sanitizers, and focused proof targets. | Neither installed. Likely available through package managers; system packages need sudo. User-local LLVM binaries or standalone cppcheck releases may avoid sudo. Establish configuration and baseline noise before gating. |

## Suggested sequence

1. Keep ASan/UBSan and focused Clang Static Analyzer runs as the low-cost baseline.
2. Add TLC for abstract state models because Java is available and installation can remain local to the audit repository.
3. CBMC 6.9.0 is selected after a bounded C++11 harness smoke test. Use it only where its parser and library model support the target unit; the direct lexer probe documents one current STL limitation.
4. Add libFuzzer for pure parser/codec boundaries only after verifying the local Clang runtime.
5. Revisit cppcheck/clang-tidy only if they reveal a concrete gap not addressed above.

Tool installation decision remains open in [PLAN.md](PLAN.md). Any locally downloaded binaries and JARs should be ignored by Git and their version/checksum recorded.
