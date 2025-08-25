# TotalSearch – Session Notes (2025-08-12)

## Context
- Investigated RGSearch performance differences between two consecutive runs.
- Stabilized JSON result rendering and improved diagnostics.

## Findings
- Log file: `build/Release/log/totalsearch.log`.
- First run timings (from log):
  - Ripgrep execution: ~1138 ms
  - JSON processing (inside function): ~417 ms
  - Wrapper timing around display exit: ~3289 ms
- Second run timings:
  - Ripgrep execution: ~242 ms
  - JSON processing: ~170 ms
  - Wrapper timing around display exit: ~172 ms
- Why second run is faster:
  - OS file cache warm (Windows page cache) → faster I/O for ripgrep.
  - UI/layout/font caches warmed → faster item creation/painting.
  - Timings are not strictly additive; the wrapper includes extra UI work beyond the inner JSON processing timer.

## Current Code Adjustments
- Reduced immediate UI repaint/update calls after bulk insert in result list to avoid extra paint/layout cost (temporarily commented).
- Added/kept detailed logging around RGSearch and JSON display.

## Where to Resume
- Open in Cursor: project root `C:/P/proj/TotalSearch` (or cloned path on new PC).
- Build: run `./build` from project root.
- Run: `build/Release/TotalSearch.exe` (launcher in scripts invokes `totalsearch.exe`).
- Logs: `build/Release/log/totalsearch.log`.

## Next Optimization Ideas (optional)
- Batch list view updates:
  - `searchResults->setUpdatesEnabled(false)` before bulk insert, `true` after.
  - `searchResults->setLayoutMode(QListView::Batched)` and tune `setBatchSize`.
  - `searchResults->setUniformItemSizes(true)`.

## Notes
- A clear between searches in app does not clear the OS page cache; to force a truly cold run, reboot or empty the standby list (e.g., RAMMap).


