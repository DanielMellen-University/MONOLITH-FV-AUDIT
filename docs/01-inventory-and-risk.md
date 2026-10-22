# Inventory and risk ranking

Risk means plausible impact multiplied by exposure and implementation complexity. Formal-methods fit describes how directly useful properties can be stated and checked. These are initial audit priorities, not defect reports.

| Rank | Component and key files | Risk | Formal fit | Why it matters |
|---|---|---:|---:|---|
| 1 | Virtual filesystem and atomic publication/cleanup: `src/fs/Filesystem.{hpp,cpp}`, `src/detail/AtomicFile.{hpp,cpp}`, `src/detail/AtomicTempOutput.cpp` | Very high | High for pure path rules; medium for syscall protocol | User paths cross into host paths. Copy, rename, deletion, symlink resolution, conditional writes, temporary workspaces, recovery, and locks have security and data-loss impact. The API is testable, but real POSIX behavior needs explicit environment assumptions. |
| 2 | Parsers and persisted/untrusted data: `TerminalLexer`, `SessionFormat.hpp`, `DesktopSettings.cpp`, `DrawingRaster.cpp`, `WallpaperImage.cpp`, file/history readers | High | High for lexer/codec; medium for input pipeline | Terminal commands, session/settings files, `.modr` content, image dimensions, text files, and host filesystem entries feed parsers and allocations. Many have pure or bounded parsing cores. |
| 3 | Window manager lifecycle and event routing: `WindowManager.{hpp,cpp}`, `src/window/detail/*`, `Window.hpp`, `App.hpp` | High | High for abstract state; medium for C++ refinement | Window pointers, focus/z-order, deferred closes, callbacks, reentrant event/update traversal, and app/controller ownership interact. Existing tests deliberately cover callback-triggered closes and reentrancy. |
| 4 | Large stateful apps and editors: `TextEditorApp`, `FilesystemApp`, `TerminalApp`, `DrawingApp` | High | Medium | They combine key/mouse input, path operations, bounded buffers/history, file lifecycle notifications, selection/undo state, and shell callbacks. Focus on invariants at their pure helper boundaries. |
| 5 | Rendering, image decode, and caches: `WindowManager::render`, app `render` methods, `WallpaperImage`, `TextTextureCache.hpp`, `RendererClip.hpp` | Medium-high | Low-medium | Coordinate conversion, clipping, pixel dimensions, cache lifetime, renderer invalidation, and third-party decoding create memory-safety and arithmetic risk. SDL behavior is an external assumption. |
| 6 | Configuration and session persistence: `DesktopSettings`, `WindowManager::loadSession/saveSession`, `SessionFormat.hpp` | Medium | High for format/model; medium for persistence | Persisted records affect restored windows and settings. Parsers are mostly compact; bounded record/window caps exist. Atomic writes are shared with rank 1. |
| 7 | Games and pure game logic: `PongLogic`, `BreakoutLogic`, Snake/Minesweeper state | Low-medium | High for isolated logic | Smaller state spaces and pure transitions are good for property-based testing and bounded exhaustive checking. App rendering remains outside that fit. |

## Input and trust boundaries

- Virtual paths supplied through Terminal, Filesystem browser, app file-open flows, clipboard-like internal path lists, settings wallpaper selection, and restored sessions.
- Host-side files and directory entries under the configured data root, including symlink and rename changes made by external processes.
- Text-editor contents, history data, desktop settings, session lines, `.modr` binary input, BMP data, and PNG/JPEG data passed through image libraries.
- SDL mouse/key/window events, integer geometry, font metrics, renderer dimensions, and system time.
- Atomic-workspace marker/token files and filesystem metadata used by cleanup and recovery.

## Ownership and concurrency map

`WindowManager` owns windows in a container; each `Window` owns its app and controller through `unique_ptr`. The controller is declared before the app so destruction order keeps it alive through the app destructor, except where close paths explicitly clear both. The manager also keeps non-owning pointers/indexes and snapshots for safe traversal during callbacks. These lifetime rules deserve state and alias invariants.

No `std::thread` or explicit pthread creation was found in `src`. The SDL event/update/render loop is the apparent single-threaded runtime. `AtomicFile.cpp` contains a process-wide `std::mutex`, `flock` locks, and cleanup tracking for inter-process atomic-write coordination. Reentrancy from app callbacks is a distinct risk and is already covered by window lifecycle tests. Verify the single-thread premise if future code introduces workers.

## Suitability summary

Best initial proof targets are the pure terminal lexer/completion helpers, `.modr` header/size arithmetic, virtual path normalization and containment predicates, and abstract window/focus transition models. Atomic publication/cleanup should start with a state model and small syscall-facing contracts; attempting to prove the entire `Filesystem.cpp` implementation at once is unlikely to be cost-effective. Rendering and full editor behavior are better served first by sanitizer, fuzz, and property testing.
