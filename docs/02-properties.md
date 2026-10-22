# Candidate properties

These are proposed obligations, not results. Add preconditions, assumptions, exact harnesses, and bounds before checking each one.

## Virtual filesystem and atomic writes

Target: `Filesystem::normalize`, `toHostPath`, `isWithinHostRoot`, `isSameOrDescendant`, `join`, `rename`, `removeRecursive`, `copyRecursive`, `writeFileWithProducerIfStampMatches`; `detail::writeAtomically` and atomic workspace helpers.

1. **Normalization is canonical and idempotent.** For every byte string accepted as a virtual path, `normalize(normalize(p)) == normalize(p)`; the result begins with `/`, has no `.` or `..` components, and has no duplicate separators except the root representation.
2. **Containment is component-aware.** For every normalized virtual path `p`, successful `toHostPath(p)` resolves within the canonical host root. A sibling name sharing a string prefix with the root must not count as contained.
3. **Root and self-descendant guards hold.** `rename` and `removeRecursive` never move/delete virtual `/`; `rename(a,b)` and `copyRecursive(a,b)` reject `b == a` and any component-wise descendant of `a`.
4. **Entry names cannot escape a directory.** `isValidEntryName(n)` is false for empty, `.`, `..`, slash-containing, and NUL-containing names; `join(dir,n)` followed by normalization remains a child only when the name is valid.
5. **Failed writes preserve the previous target.** Under the documented lock and filesystem assumptions, if a producer fails, writing/sync/publish fails, or the expected file stamp conflicts, the old target bytes remain unchanged and no partial target becomes visible.
6. **Successful publication is complete and atomic.** A successful write exposes exactly the producer output. For expected-absent writes, an existing target is not replaced; for stamp-conditional writes, publication proceeds only if the target and pinned parent still match the expected stamp/identity at the final check.
7. **Cleanup does not remove live or unowned state.** `tryScavengeAtomicTempWorkspace` removes only workspaces that satisfy the strict owner/type/readiness/lease predicates while holding the parent lock. A held lease or any unknown entry makes the workspace non-removable.
8. **Chunk progress is bounded.** `maintenanceStep(entryBudget)` and scheduled atomic sweeps inspect no more than the declared entry budget per step, preserve/restart cursors as documented, and eventually finish under finite readable directory trees and no permanent lock contention.

The last liveness property requires a finite input tree and a fairness assumption for filesystem access/lock acquisition. Symlink retargets and host writers that ignore the locking protocol must be modeled explicitly.

## Terminal and persisted-data parsers

Target: `tokenizeCommandLine`, `completionContextAt`, `escapeCompletion`, `session::readPath/writePath`, `DesktopSettings` parsers, `DrawingRaster` codec.

1. **Lexer termination and bounds.** For any finite `line` and cursor, `tokenizeCommandLine` and `completionContextAt` terminate after O(n) scanning; cursor is clamped to `line.size()`; no indexing occurs outside `[0,line.size())`.
2. **Lexer rejects malformed quotes atomically.** Unterminated single/double quotes and a trailing unquoted escape produce a non-empty error and no partial argv. Well-formed inputs return tokens consistent with the documented quoting/escape rules.
3. **Completion round trip.** For valid token text `v` and quote mode `q`, inserting `escapeCompletion(v,q)` in the corresponding token context and tokenizing reconstructs `v`, subject to the caller's insertion-context preconditions. This deserves exhaustive bounded tests over a small alphabet plus fuzzing over bytes.
4. **Session codec round trip and limits.** For strings in the supported path domain, `readPath(writePath(p)) == p`; malformed/truncated quotes fail without accepting a partial path; `loadSession` reads at most `kMaxSessionRecords` records and restores at most `kMaxRestoredWindows` windows.
5. **Settings parser totality.** `DesktopSettings::load` handles every finite input byte sequence without throwing out of the API, leaves defaults for invalid values, and only accepts RGB, boolean, and scale values within documented ranges.
6. **`.modr` dimension arithmetic is safe.** Decoder validates width/height, multiplication overflow, pixel count, encoded-byte cap, exact payload length, and channel/buffer sizes before allocation/indexing; rejected input leaves destination state unchanged.

## Window manager and ownership

Target: `WindowManager::createWindow`, `closeWindow`, `processDeferredCloses`, `update`, `handleEvent`, `bringToFront`, `focusTopmostVisibleWindow`, `cycleFocus`, `isLiveWindow`, and path notification/binding routines.

1. **Ownership/index consistency.** Every non-null pointer in focus, drag, capture, deferred-close, binding, and app-instance tracking state refers to a currently owned live window or is explicitly cleared before destruction. Owned windows have unique IDs.
2. **Focus coherence.** At most one live, non-minimized window is focused; if one exists, it is topmost under the manager's defined z-order. Closing or minimizing the focused window transfers focus to a live eligible window or leaves focus null.
3. **Deferred close safety.** A window is not destroyed during an app callback traversal that still holds its pointer; queued close requests are applied after the relevant traversal snapshot unwinds. Repeated close requests are idempotent.
4. **Callback reentrancy.** A callback may close itself, close a sibling, create a window, or issue nested layout/path notifications without a later dereference of a destroyed or stale pointer.
5. **Geometry constraints.** For valid desktop geometry and resize requests, minimum window sizes and clamping invariants hold without signed overflow; hit-test coordinates map to the documented logical client region.

A finite TLA+ model can exhaustively check lifecycle/focus/close invariants for small window counts and a bounded callback action set. Refinement to C++ requires tracing each abstract transition to the named implementation method and tests.

## File-backed apps, rendering, and other apps

Target: `TextEditorApp` file loading/edit/save helpers, `FilesystemApp` selection/filter/paste state, `TerminalApp` bounded history/scrollback, `DrawingRaster`, wallpaper image dimensions, rendering caches, and game logic.

- Opening/replacing an editor document respects the 16 MiB and 65,536-line caps and does not replace the current document after a rejected open.
- Find/replace, undo/redo, and drawing raster updates preserve document/canvas invariants and stay inside tracked byte/state budgets.
- `WallpaperImage` rejects non-positive or excessive dimensions before allocation, with `width * height <= 16,777,216` checked without overflow.
- Renderer cache eviction never leaves a texture used after its renderer is destroyed; renderer/font/scale changes invalidate all dependent textures before use.
- Pure game transitions preserve board bounds and game-specific score/life/terminal-state invariants for all states in the declared finite domain.

For app behavior, begin with property-based tests, mutation/fuzz inputs, and sanitizers. Select only small pure transitions for bounded proofs.
