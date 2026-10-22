# Scope and method

## Run 1 boundary

The audit target is MONOLITH `main` (stable) at `446e107efa466c47dbd5aa48333da1d7e879ede6`, release tag `v1.0.1` (2026-10-22 baseline). At baseline time, `beta` pointed to that same commit, so the baseline results carry over unchanged. Later audit re-pins follow `main` only. The project is a single-process C++23/SDL2 desktop application with a virtual filesystem and nine built-in apps. This run maps risk, names candidate properties, assesses local tooling, and captures reproducible build/test evidence. It does not change MONOLITH or claim proofs.

All audit documents, specifications, extracted harnesses, scripts, and reports are kept in this audit repository. Source builds and test outputs are placed outside the target checkout. The target is read-only for this audit.

## Proposed method

1. Pin every result to the target commit and state compiler, flags, dependencies, test inputs, bounds, and assumptions.
2. Start with executable specifications for compact algorithms and state machines. Prefer boundary-focused properties over vague claims such as “memory safe”.
3. Use bounded checking on extracted pure C++ units with explicit input bounds. Check that loop unwinding is complete before calling a result a proof for that bounded harness.
4. Use TLA+ or Alloy for bounded abstract state exploration of window lifecycle, focus, filesystem publication, and cleanup protocols. A model result applies to the model and its declared state bounds, not automatically to the C++ implementation.
5. Use compiler analysis, sanitizers, fuzzing, and property tests as supporting bug-finding evidence. Record the exact analyzed translation units and test coverage.
6. Review counterexamples and warnings against the code. Keep confirmed defects separate from unreviewed tool reports.

## Scope exclusions and assumptions

- SDL2, SDL_ttf, the C++ standard library, the OS kernel, and third-party `stb_image` are dependencies, not subjects for proofs in this audit.
- The user-visible application event loop appears single-threaded. Atomic-file cleanup also uses a process mutex and POSIX `flock` for inter-process coordination. Host programs that do not honor MONOLITH's locks may race with publication; this is a stated environmental assumption, not an established guarantee.
- Callback-driven reentrancy in `WindowManager` is in scope even without concurrent application threads.
- Results from bounded tools cover only their harness, model, environment, and declared bounds. Dynamic testing samples executions and does not establish absence of defects.

## Evidence labels

- **Build/test evidence:** the named command completed with the recorded result.
- **Static analysis:** a named analyzer reported its findings for the selected translation units and options.
- **Bounded check:** a model checker found no counterexample within a stated bounded harness and discharged unwinding checks where applicable.
- **Model check:** a state-space checker exhausted the stated finite model.
- **Proof:** reserve this for an actual proof obligation discharged by a sound tool under listed assumptions. Never infer it from test success or a clean analyzer run.
