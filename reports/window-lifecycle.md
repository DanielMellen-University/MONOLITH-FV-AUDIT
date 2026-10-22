# Window lifecycle model

## What is checked

[`specs/window/WindowLifecycle.tla`](../specs/window/WindowLifecycle.tla) is a finite abstract state model motivated by `WindowManager::createWindow`, `closeWindow`, `processDeferredCloses`, `update`, `focusTopmostVisibleWindow`, and the `AppCallbackScope` callback-depth guard in `src/window/WindowManager_private.inc`.

The model explores creating windows, raising/restoring/minimizing them, entering nested app callbacks, requesting close during callbacks, returning from callbacks, and processing deferred close requests after the outermost callback returns.

## Properties

- `TypeOK`: ownership/order consistency; focus and minimized state refer to known live windows; deferred requests refer to live windows; callback frames contain live window identities; callback nesting stays within the declared bound.
- `FocusInvariant`: focus is exactly the topmost non-minimized live window, or `None` if no visible live window exists.
- The callback-owner portion of `TypeOK` checks that a window referenced by any active callback frame is not removed from `Live`. A close request made while any callback is active adds the target to `Pending`; processing is enabled only when the callback stack is empty.

## Bounds and assumptions

- TLC constants: `Windows = {w1,w2}`, `None = none`.
- At most two unique window identities can ever be created. `Seen` prevents identity reuse after close.
- Callback stack depth is bounded at two nested frames. This includes nested callback behavior only to that depth.
- Each window is modeled as one app owner. Start-menu suspension, host focus loss, pointer capture, close vetoes (`App::allowClose`), app/controller destructor ordering, path bindings, and SDL events are abstracted away.
- A close with callback depth zero is modeled as immediately allowed. `allowClose` veto/reentrancy behavior is outside this model.
- Focus is defined as topmost visible order. No fairness or liveness claim is checked.
- Deadlock checking is disabled because the finite model intentionally reaches a terminal state when both unique identities have been created and closed. The invariants remain enabled over all reachable states.
- The spec is an abstract model, not a refinement proof of the C++ implementation.

## Reproduction

From the audit repository root, with TLC `2026.10.20.183001` / TLA+ tools `v1.8.0` already installed and OpenJDK `11.0.32.1`:

```sh
timeout 180s java -XX:+UseParallelGC -jar tools/tla2tools.jar \
  -workers 1 \
  -metadir /tmp/monolith-fv-window-lifecycle-run3 \
  -config specs/window/WindowLifecycle.cfg \
  specs/window/WindowLifecycle.tla
```

The time box is 180 seconds. TLC metadata and traces are written under `/tmp`; the spec and configuration are the only audit inputs.

## Result

Initial runs exposed model setup issues: TLC does not provide the assumed `IndexOf`/`SetOfSeq` operators directly, so the model now defines them and extends `FiniteSets`; TLC's default deadlock check reported the intended saturated terminal state, so the config disables only deadlock checking. A later counterexample showed that the abstraction allowed a new direct close to race the deferred-close queue after callback return. The actual callback scope flushes queued closes synchronously, so the model now disallows that interleaving by requiring the queue to be empty before an outside-callback direct close.

Final run result: TLC `2026.10.20.183001` exited 0 and reported `Model checking completed. No error has been found.` It checked `TypeOK` and `FocusInvariant`, with 2,061 states generated, 276 distinct states, depth 9, and zero states left on the queue. It completed in under one second. These invariants are verified for this complete finite model only, under the bounds and assumptions above. This is not a proof of the C++ implementation.
