# Results index

This index distinguishes tool-checked evidence from formal proof. “Checked” means the named run completed on its recorded inputs. “Verified” is reserved for a tool that proved a stated property under recorded bounds and assumptions.

| ID | Artifact or check | Target pin | Result | Evidence |
|---|---|---|---|---|
| BASE-01 | GCC/CMake Debug build and registered headless test suite | `main` / `v1.0.1` / `446e107efa466c47dbd5aa48333da1d7e879ede6` | Checked; build and all 34 tests passed. Not a proof. | [Baseline results](04-baseline-results.md) |
| BASE-02 | ASan/UBSan application build and headless test suite | `main` / `v1.0.1` / `446e107efa466c47dbd5aa48333da1d7e879ede6` | Checked exercised paths; suite passed without sanitizer reports. Not a proof. | [Baseline results](04-baseline-results.md) |
| BASE-03 | Warning-heavy GCC build | `main` / `v1.0.1` / `446e107efa466c47dbd5aa48333da1d7e879ede6` | Checked; build passed with 269 diagnostics, mostly conversions and generated dependency warnings. | [Baseline results](04-baseline-results.md) |
| BASE-04 | Clang Static Analyzer on six selected translation units | `main` / `v1.0.1` / `446e107efa466c47dbd5aa48333da1d7e879ede6` | Checked; no diagnostics for those translation units. Not a proof. | [Baseline results](04-baseline-results.md) |

Append one row per completed harness, model, tool smoke test, fuzz run, or other check. Give each a stable ID and link to a report that records the exact property, scope, bounds, assumptions, command, tool version, and result.
