# Window lifecycle model-to-code review

## Target and source map

Target: MONOLITH `main` (stable), release `v1.0.1`, commit `446e107efa466c47dbd5aa48333da1d7e879ede6`.

| Model behavior | C++ implementation path | Existing regression coverage |
|---|---|---|
| Window identities enter and leave the owned set | `createWindow`, `closeWindow`, `m_liveWindowIds`, `isLiveWindow` in `WindowManager` and `wm_body_01a.inc`, `wm_body_05.inc`, `wm_body_06.inc` | Self-close during update and identity-index removal in `scripts/test_window_lifecycle.cpp` |
| Callback depth delays destructive close | `AppCallbackScope` in `WindowManager_private.inc`; `closeWindow` queues unique pointer/ID pairs while callback depth is nonzero; outermost scope calls `processDeferredCloses` | Update, nested layout, observer notification, and bound-file callback close cases in `scripts/test_window_lifecycle.cpp` |
| Nested close requests do not reuse stale storage | `processDeferredCloses`; `closeWindow` rechecks `isLiveWindow` after `allowClose` and focus-loss callbacks and reacquires its vector iterator after reentrant callbacks | Reentrant `allowClose`, focus-loss self-close, and focus-loss sibling-close cases in `scripts/test_window_lifecycle.cpp` |
| Focus is transferred after a focused window closes | `focusTopmostVisibleWindow`, `bringToFront`, and focused-window cleanup in `wm_body_05.inc` and `wm_body_06.inc` | Focus gain/loss handoff cases in `scripts/test_window_lifecycle.cpp` and `scripts/test_window_focus.cpp` |

This is a manual traceability review plus dynamic regression evidence. It is not a refinement proof between the C++ implementation and the TLA+ model. The TLA+ model abstracts start-menu suspension, host focus loss, SDL events, app/controller destruction details, path bindings, and `App::allowClose` veto behavior. Its checked callback depth is at most two and it has at most two unique window identities.

## Focused regression check

The existing `scripts/test_window_lifecycle.cpp` binary from the pinned baseline build was run directly in both existing build trees. GCC used for the baseline build is `11.4.0`; the test binary reports all assertions passed.

Normal run command from MONOLITH root:

```sh
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy ./build/test_window_lifecycle
```

Sanitized run command from MONOLITH root:

```sh
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=halt_on_error=1 \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
  ./build-sanitized/test_window_lifecycle
```

Both exited 0 and printed `ALL WINDOW LIFECYCLE TESTS PASSED`. The tests cover the named callback reentrancy, queued close, survivor, and focus handoff cases. ASan/UBSan emitted no report. This is checked behavior for those regression traces, not a proof of memory safety.

## TLA+ model result

The separate [window lifecycle report](window-lifecycle.md) records TLC `2026.10.20.183001`, TLA+ tools `v1.8.0`, OpenJDK `11.0.32.1`, and the exact reproduction command. TLC exhaustively checked `TypeOK` and `FocusInvariant` over 276 distinct states, two unique identities, and callback depth at most two. Those properties are verified for that finite abstract model only.
