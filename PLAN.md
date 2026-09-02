# FOX Toolkit Integer Pixel Scaling — Implementation Plan

## Status (2026-09-02)

Branch: `feature/integer-pixel-scaling`.

- **Phase 1 (done, commit `691e583`):** `FXApp::scale` + `-scale N` flag;
  geometry scaling in `FXWindow::create/move/position/resize`; drawing scaling
  in `FXDCWindow::fillRectangle`/`drawText`; input unscaling for mouse and
  `ConfigureNotify`. Tested on Pathfinder at scale=2.
- **Phase 2 (in progress, commit `0f7c145`):** `FXTopWindow::move/resize/position`
  now scale WM_NORMAL_HINTS and the X11 calls they send (Phase 1's
  `FXWindow` scaling doesn't apply to top-level windows, which override
  these); raw Expose events unscaled at both entry points
  (`getNextEvent`, `removeRepaints`).
  - **Found and fixed a real bug**: Phase 1's font-size scaling (multiplying
    the Xft resolution) inflated `FXFont`'s own metric queries
    (`getFontHeight`, `getTextWidth`, ...), which is what logical layout math
    (`getDefaultWidth/Height`) is built on — so text-influenced widgets got
    double-scaled (once via inflated metrics, again at the geometry
    chokepoint) while others stayed single-scaled, corrupting layout badly
    enough that large parts of the window were never painted. Reverted the
    font-size hack; confirmed the window now paints completely at scale=2
    (verified against a scale=1 window manually resized to the same physical
    size). Text is back to unscaled (logical) size until glyph scaling is
    redone properly.
  - **Drawing primitives (commit `389e621`)**: scaled the remaining
    `FXDCWindow` X11 primitives — `drawPoint`, `drawLine`, `drawRectangle`,
    `drawRoundRectangle`, `drawArc`, `drawEllipse`, `fillRoundRectangle`,
    `fillChord`, `fillArc`, `fillEllipse` (scalar geometry); destination
    positions for `drawArea`/`drawImage`/`drawBitmap`/`drawIcon*`
    (icon/image pixel content itself still native size — see item 4 below);
    `drawHashBox`/`drawFocusRectangle` (raw `XFillRectangle` calls, bypassed
    the already-scaled `fillRectangle`); `setLineWidth`; tile/stipple/clip-mask
    origins.
    - **Found and fixed a second real bug**: `clip`/`rect` are kept in
      *logical* pixels throughout `FXDCWindow.cpp` (derived from
      `FXDrawable::getWidth/Height`), but every
      `XSetClipRectangles`/`XftDrawSetClipRectangles` call handed that
      logical rectangle to X11/Xft directly, which expects physical pixels —
      under-clipping (to a too-small physical region) any paint that goes
      through `FXDCWindow(draw,event)`, the constructor essentially every
      widget's `onPaint` uses, whenever a widget also narrows its own clip
      (e.g. scrolled lists/trees/text via `setClipRectangle`). Added a
      `scaledClipRect()` helper and applied it at every such call site.
    - Verified on Pathfinder at scale=2 via a fresh `xwd` capture (`import
      -window` proved unreliable/stale in this environment — prefer `xwd` +
      `convert` for screenshots here): full window now paints completely and
      correctly — button bevels, split-pane sash, scrollbar, and tree lines
      all scale consistently. Icons remain native pixel size (expected).
  - **Still open in Phase 2**:
    1. Array/batch `FXDCWindow` primitives not scaled (`drawPoints`,
       `drawLines`, `drawLineSegments`, `drawRectangles`, `drawArcs`,
       `fillArcs`, `fillChords`, the `fillPolygon*` family) — lower priority,
       used mostly by custom canvas drawing rather than standard widget
       chrome, would need a scaled temporary copy of the caller's array.
    2. Icon/image pixel content itself isn't resampled — `FXImage`/`FXBitmap`/
       `FXIcon` need a nearest-neighbor upscale path (item 4 of the original
       Phase 2 list).
    3. `FXWindow` reparent path untouched.
    4. Broader input/event translation (drag, resize handles, scroll
       regions) not yet systematic — only mouse buttons/motion and
       Configure/Expose are unscaled so far.
    5. Only tested on Pathfinder, not multiple example apps.
  - **Glyph-size scaling needs redoing**: a font that renders bigger without
    lying about its logical metrics — e.g. a second, physically-scaled
    `XftFont` used only for drawing, while the font object's metric-query
    methods stay backed by the logical-size font. See Phase 2 item 5 / Phase 3.

## High-Level Recap

We're building **integer pixel scaling into the FOX Toolkit itself**, so that any FOX
application (starting with a Pathfinder-based app using the React95 bitmap font) can
render at a clean 2x, 3x, 4x, etc. — with **every** element scaling together: fonts,
buttons, borders, padding, icons, scrollbars, menus — not just the font.

The mechanism: FOX's internal widget/layout code keeps working in **logical pixels**,
completely unchanged. A new global integer scale factor (`FXApp::scale`) is introduced,
and the actual multiplication into **physical pixels** happens only at a small number of
chokepoints where FOX talks to X11 — window creation, drawing calls, and input events.
Bitmap font support is added as one piece of this system (for pixel-perfect glyph
scaling), not as a separate standalone feature.

This replaces the current working-but-hacky solution
(`XPRA_SCALING_FILTER=nearest run_scaled --scale=2`) with a native, toolkit-level
feature that benefits every FOX app, with no external tool dependency.

### Background context

- Original goal: render a FOX Toolkit app at 2x scale.
- This only worked via Xpra's `run_scaled` combined with
  `XPRA_SCALING_FILTER=nearest` (confirmed working, but felt hacky/slow).
- Investigated modifying FOX Toolkit directly to support scaling natively.
- Initial idea was "just add bitmap font support" — but the real requirement is
  broader: **all controls and elements** need to scale in integer steps, not just
  fonts. Bitmap fonts are one component of the larger scaling system.

### Core architecture

```
FOX application  ->  logical coordinates/sizes  ->  FOX toolkit (x N)  ->  X11 physical pixels
```

Example at scale=2:
- 10x10 button   -> 20x20
- 1px border     -> 2px
- 8px padding    -> 16px
- 12px font      -> 24px
- 16px icon      -> 32px
- 800x600 window -> 1600x1200

Integer-only scaling is a deliberate constraint (`FXint scale = 1, 2, 3, 4...`) —
no 1.25x/1.5x/1.75x fractional scaling, so every logical pixel maps to exactly N
physical pixels with no rounding or blur.

### The three boundaries where scaling is applied

1. **Geometry — `FXWindow::create()`**
   Where widget geometry becomes an actual X11 window (`XCreateWindow(...)`).
   FOX's internal geometry model stays logical; multiplication happens right
   before the X11 call:
   ```cpp
   physical_x = logical_x * scale;
   physical_y = logical_y * scale;
   physical_w = logical_w * scale;
   physical_h = logical_h * scale;
   ```

2. **Drawing — `FXDCWindow`**
   Every drawing primitive (`drawRectangle`, `fillRectangle`, `drawLine`, `drawArc`,
   `drawEllipse`, `drawText`, `drawImage`, `drawBitmap`, `drawIcon`, `drawArea`)
   routes through this one class. The DC transforms coordinates before issuing the
   X11 call, e.g. `drawRectangle(10,10,100,30)` -> `XDrawRectangle(...,20,20,200,60)`.
   Line widths (borders) scale the same way.

3. **Input/events — reverse direction**
   X11 reports mouse position in physical pixels; FOX needs it back in logical
   pixels, or clicks would land in the wrong place relative to the visually
   larger UI:
   ```
   mouse = (423,187) physical  ->  mouse = (211,93) logical
   ```
   The transform is bidirectional: logical->physical for geometry/rendering,
   physical->logical for input/events (mouse, drag, resize handles, expose
   regions).

### Why this needs relatively little code

Widget constants stay as-is in the source:
```cpp
#define DEFAULT_PAD 4
#define DEFAULT_BORDER 1
```
The toolkit applies the scale at the geometry/drawing boundary, so these become
8 and 2 physically **without editing every widget's numbers by hand**. Layout
math (`getDefaultWidth()`, `getDefaultHeight()`) also stays entirely logical — a
button computed as logical 60x16 just becomes physical 120x32 at render time.

### Where bitmap fonts fit

Bitmap fonts slot into the **rendering** branch of this same scaling system,
alongside "primitives" and "images":
```
FXApp
  |-- scale = 2
       |-- geometry (x2 physical)
       `-- rendering (x2 physical)
             |-- primitives -> integer scale
             |-- images     -> nearest neighbor
             `-- fonts      -> bitmap / Xft
```
A bitmap font backend sits underneath `FXFont` (satisfying the same `drawText()`,
`getFontHeight()`, `hasChar()` contract Xft-based fonts already do), so a 9x16
pixel glyph cleanly becomes an 18x32 glyph at scale 2 — pixel-doubled, no
interpolation — while everything around it (buttons, padding, icons, scrollbars)
scales through the same geometry/drawing boundary.

### Effort estimate

| Scope                              | LOC          | Files touched |
|-------------------------------------|--------------|---------------|
| Minimal prototype                   | 150-300      | `FXApp::scale`, basic window scaling, one `FXDCWindow` path, mouse unscaling, one scaled font |
| **Full FOX-wide implementation (target)** | **500-1,000** | `FXApp`, `FXWindow`, `FXDCWindow`, `FXFont`, `FXImage`/`FXBitmap`/`FXIcon`, X11 event handling |
| Polished/production                 | 1,000-2,000  | + clipping, cursors, drag-and-drop geometry, popups, multi-monitor, GL canvases, tests, docs |

The 500-1,000 LOC tier is realistic because FOX already centralizes fonts through
`FXApp::getNormalFont()` and drawing through `FXDCWindow` — the scale factor only
needs to be applied at those chokepoints, not threaded through every individual
widget class.

---

## Implementation Plan

### Phase 0 — Baseline & safety net
- Confirm the project builds from source and Pathfinder (or target app) runs
  unscaled, as a regression baseline.
- Create a feature branch in the `fox-src` repo.
- Keep the Xpra `--scale=2` + `XPRA_SCALING_FILTER=nearest` workaround as a
  fallback throughout development — don't break the working setup.

### Phase 1 — Minimal prototype (~150-300 LOC)
Goal: prove the concept end-to-end on one widget, not full coverage.

1. Add `FXint scale = 1;` to `FXApp`, plus `getScale()` / `setScale(FXint)`.
2. **Geometry**: in `FXWindow::create()`, multiply `xpos/ypos/width/height` by
   `scale` right before the `XCreateWindow` call.
3. **Drawing**: in `FXDCWindow`, scale coordinates in just one or two primitives
   (e.g. `fillRectangle`, `drawText`) as a proof point.
4. **Input**: divide incoming X11 mouse coordinates by `scale` before FOX
   widgets see them.
5. **Font**: hardcode a scaled font size (`fontsize * scale`) via Xft — no
   bitmap font yet.
6. Test on the target app (or a tiny throwaway FOX app) at `scale=2` — confirm
   the window is 2x bigger, mouse clicks land correctly, and a couple of
   widgets render at the right size/position.

**Exit criteria:** a real window, correctly positioned/sized, that responds
correctly to mouse input, at 2x.

### Phase 2 — Full FOX-wide geometry & drawing (~500-1,000 LOC, main target)
Expand Phase 1 to cover the toolkit systematically.

1. **`FXWindow`**: apply scaling consistently across all geometry paths
   (create, resize, move, reparent) — not just initial creation. Ensure
   parent/child coordinate consistency.
2. **`FXDCWindow`**: scale *every* drawing primitive — `drawRectangle`,
   `drawLine`, `drawArc`, `drawEllipse`, `drawImage`, `drawBitmap`, `drawIcon`,
   `drawArea` — plus line widths (1px border -> Npx).
3. **Input/events**: make the physical->logical mouse/expose-region
   translation systematic (drag, resize handles, scroll regions — not just
   clicks).
4. **`FXImage`/`FXBitmap`/`FXIcon`**: decide nearest-neighbor scaling behavior
   for existing raster images/icons at integer scale.
5. **`FXFont`**: scale Xft font sizes as the default path (still no bitmap
   fonts yet) — validates that widget layout math (`getDefaultWidth/Height`)
   correctly stays logical while physical rendering comes out right.
6. Test broadly: run several FOX example apps (not just the target app) at
   scale=2 and scale=3, checking menus, dialogs, scrollbars, tooltips.

**Exit criteria:** any unmodified FOX app runs correctly and proportionally at
an integer scale, using standard Xft fonts.

### Phase 3 — Native bitmap font support (~300-700 LOC)
Layer in pixel-perfect fonts as an `FXFont` backend.

1. Design a simple bitmap font format/representation (either a native format,
   or an importer for existing bitmap-derived TTFs like React95).
2. Implement the backend so it satisfies the existing `FXFont` contract:
   `getFontAscent/Descent/Height/Width`, `getTextWidth`, `hasChar`, `drawText`.
3. Wire it into `FXFont::drawText()`: branch to a bitmap glyph renderer instead
   of `XftDrawStringUtf8()` when a bitmap font is active.
4. Render each glyph as integer NxN pixel blocks (no antialiasing/
   interpolation), respecting the same global `scale` from Phase 2 — a 9x16
   glyph becomes 18x32 at scale 2.
5. Test with React95 (or another bitmap-derived font) at scale=1, 2, 3 —
   confirm crisp, non-smoothed output that matches the
   `XPRA_SCALING_FILTER=nearest` result, without needing Xpra.

**Note on test fonts:** React95 is a TTF *derived from* a bitmap font, not an
actual bitmap font — it may already be smoothed/interpolated at the outline
level, which would mask whether our NxN pixel-block rendering is doing real
integer scaling or just riding on top of font-level antialiasing. For a
trustworthy scale=2 test, use a genuine bitmap font — e.g. a classic Windows
`.FON` file (bitmap `MS Sans Serif`, or similar) — so pixel-doubling can be
verified against a source that has no smoothing to hide behind. Worth
revisiting when Phase 3 actually starts; not a blocker for Phase 2.

**Exit criteria:** the target app renders with a genuine bitmap font as
pixel-sharp glyphs, fully integrated with the scale system from Phase 2 — no
Xpra involved.

### Phase 4 — Polish (~1,000-2,000 LOC total, incremental / as-needed)
Only tackle these once Phases 1-3 work and the scaling system is in daily use:
- Clipping regions, cursors, drag-and-drop geometry, popup/menu positioning at
  scale.
- Multi-monitor / per-monitor scale handling.
- GL canvas considerations if any FOX apps use OpenGL.
- Tests, docs, a config/preferences entry for the scale setting.

---

## Suggested Order of Work

1. **Phase 1** first — validates the geometry/drawing/input triangle actually
   works before investing in full coverage.
2. **Phase 2** next — the bulk of the effort and the main enabler for
   "everything scales."
3. **Phase 3** (bitmap fonts) after Phase 2 is solid — depends on the scale
   factor already existing.
4. **Phase 4** only as real needs come up — don't front-load it.
