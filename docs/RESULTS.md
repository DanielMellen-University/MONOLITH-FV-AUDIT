# Results index

This index distinguishes tool-checked evidence from formal proof. “Checked” means the named run completed on its recorded inputs. “Verified” is reserved for a tool that proved a stated property under recorded bounds and assumptions.

| ID | Artifact or check | Target pin | Result | Evidence |
|---|---|---|---|---|
| BASE-01 | GCC/CMake Debug build and registered headless test suite | `main` / `v1.0.1` / `446e107efa466c47dbd5aa48333da1d7e879ede6` | Checked; build and all 34 tests passed. Not a proof. | [Baseline results](04-baseline-results.md) |
| BASE-02 | ASan/UBSan application build and headless test suite | `main` / `v1.0.1` / `446e107efa466c47dbd5aa48333da1d7e879ede6` | Checked exercised paths; suite passed without sanitizer reports. Not a proof. | [Baseline results](04-baseline-results.md) |
| BASE-03 | Warning-heavy GCC build | `main` / `v1.0.1` / `446e107efa466c47dbd5aa48333da1d7e879ede6` | Checked; build passed with 269 diagnostics, mostly conversions and generated dependency warnings. | [Baseline results](04-baseline-results.md) |
| BASE-04 | Clang Static Analyzer on six selected translation units | `main` / `v1.0.1` / `446e107efa466c47dbd5aa48333da1d7e879ede6` | Checked; no diagnostics for those translation units. Not a proof. | [Baseline results](04-baseline-results.md) |
| TOOL-TLC-01 | TLC local installation and CLI launch check | Tool setup, not a MONOLITH source check | Checked; TLC `2026.10.20.183001` prints usage/version with Java 11. No MONOLITH property checked. | [Tool smoke tests](tool-smoke-tests.md#tlc-cli-launch) |
| TOOL-TLC-02 | TLC finite-model smoke | Toy model only, not a MONOLITH source check | Verified `TypeOK` in the complete finite `n ∈ 0..2` model; 3 distinct reachable states. | [Tool smoke tests](tool-smoke-tests.md#tlc-finite-model-smoke) |
| TOOL-CBMC-01 | CBMC local install and C++ harness smoke | Toy harness only, not MONOLITH source | Verified three assertions for all six assumed index values `-1..4`, count fixed at 4, no loops. | [Tool smoke tests](tool-smoke-tests.md#cbmc-c-assertion-harness) |
| MODEL-WM-01 | Window ownership, callback deferral, and focus model | Abstract finite model of `WindowManager`; two identities and callback depth at most two | Verified `TypeOK` and `FocusInvariant` over the exhausted 276-state graph. Model only, not C++ refinement. | [Window lifecycle report](../reports/window-lifecycle.md) |
| MODEL-FS-01 | Atomic workspace publication and cleanup model | Abstract finite model of one writer, one cleaner, and at most two content chunks | Verified five invariants over the exhausted 34-state graph. Model only, not C++ refinement; POSIX assumptions and exclusions apply. | [Atomic workspace report](../reports/atomic-workspace.md) |

Append one row per completed harness, model, tool smoke test, fuzz run, or other check. Give each a stable ID and link to a report that records the exact property, scope, bounds, assumptions, command, tool version, and result.
