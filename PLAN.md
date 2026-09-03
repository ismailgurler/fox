# FOX Toolkit Integer Pixel Scaling — Implementation Plan

## Status (2026-09-03, even later — "Before Phase 4")

Four follow-ups the user asked for explicitly, all done:

1. **Registry-backed bitmap font search path.** `FXFont::listBitmapFonts()`/
   `isBitmapFontPath()`/`defaultBitmapFontPath` and `FXBitmapFontEntry`
   promoted from lib-internal `lib/xfntface.h` to the public
   `include/FXFont.h` (`4b6797c`) -- needed once `FXFontSelector` (item 3
   below) started consuming them directly. New `SETTINGS/bitmapfontpath`
   registry entry (default `/usr/local/share/fonts:~/.local/share/fonts`,
   `FXFont::defaultBitmapFontPath`) is the single source of truth for
   where to scan, editable via a new "Bitmap Font Path" field on
   ControlPanel's Themes tab -- mirrors "Icon Search Path" exactly
   (colon-separated, `~`-expanding, same widget/FXDataTarget/load-save
   pattern). Both `BitmapFontDialog` and `FXFontSelector` read it, falling
   back to the default.
2. **`SETTINGS/scale` persistence -- verified, no fix needed.** Launched
   ControlPanel with no `-scale` flag; it opened at physical 2x with the
   spinner correctly showing "2", read straight from a `scale=2` a
   previous session had saved to `~/.config/fox.rc`. Works as designed.
3. **The real `FXFontSelector` merge, `b269a8a`.** Bitmap fonts now show
   up in the *shared* font-selection widget (behind `FXFontDialog`),
   alongside Xft ones in one alphabetically-sorted list -- so any FOX app
   using the standard font dialog gets bitmap-font picking for free, not
   just ControlPanel's dedicated `BitmapFontDialog`. This was the
   originally-planned approach, deferred earlier this session as "too
   invasive" in favor of the ControlPanel-only dialog (`72666e0`) --
   revisited because e.g. Adie had no other way to pick a bitmap font.
   - `familylist` merges Xft families (`FXFont::listFonts()`) with unique
     bitmap-font family names (`FXFont::listBitmapFonts()`) into one
     `FXList::ascending`-sorted list. Bitmap entries are tagged with a
     `BITMAP_FAMILY_MARKER` item-data sentinel (all-bits-set -- can't
     collide with an Xft entry's data, always a small `FXFont` flags
     combination) so `onCmdFamily()`/`listFontFaces()` can tell the two
     kinds apart regardless of where sorting places them.
   - `selected.face` holds a real `.fon` file path whenever the current
     pick is a bitmap font (a bitmap "family" has no single loadable
     name -- Windows ships separate files per weight/style, unlike Xft
     where one family name covers everything). New `bitmapmode`/
     `bitmapfamily` members, re-derived in `listFontFaces()` from whether
     `selected.face` is a `.fon` path (`FXFont::isBitmapFontPath()`),
     drive bitmap-specific branches in `listWeights()`/`listSlants()`/
     `listFontSizes()` (filter the flat scan by family → weight → slant
     instead of an Xft query) and `previewFont()` (resolve `selected.face`
     to the exact file for the chosen weight/slant/size via
     `resolveBitmapPath()` -- may differ from the file `onCmdFamily()`
     seeded). `onCmdWeight`/`onCmdStyle`/`onCmdSize` needed *no* changes:
     they already just read back list-item data in units
     (`FXFont::Normal`/`Bold`, `Straight`/`Italic`, deci-points) the
     bitmap branches now also populate.
   - New `updateFilterEnabled()` greys the five Xft-only controls
     (Character Set/Set Width/Pitch/Scalable/All Fonts) in bitmap mode --
     same `isBitmapFont()`-driven pattern as ControlPanel's
     `updateFontControlsEnabled()`.
   - Verified end-to-end in Adie (Options > Font...): "Fixedsys" (from
     the test `vgafix.fon`) appears correctly interleaved alphabetically
     among Xft families; selecting it shows the one real weight/style/
     size combination that file actually has (normal/regular/12.0); the
     live preview renders genuine bitmap glyphs; the five Xft filter
     controls grey out; picking an Xft font again re-enables them; Accept
     applies the bitmap font to Adie's actual text editing area (screenshot-
     confirmed crisp pixel glyphs in typed text). Also confirmed no
     regression in ControlPanel's own "Choose Font..." (same
     `FXFontDialog`), including at scale=2.
4. **Hairline-border scale bug fixed everywhere else, `1a34896`.** Same
   bug as the `FXFrame`/`FXToolTip` fix (`455c61c`): a stroked
   `dc.drawRectangle()` is an X11 hairline, always 1 physical pixel
   regardless of scale. Audited every other `dc.drawRectangle()` call in
   the toolkit; fixed the genuine chrome/content borders (`FXPopup`,
   `FXPacker`, `FXToolBarShell` -- each had their own copy of `FXFrame`'s
   exact bug; `FXDial`, `FXCheckButton`, `FXColorList`, `FXFoldingList`/
   `FXTreeList`, `FXMDIButton`, `FXMenuCheck`, `FXRulerView`, `FXText`'s
   overstrike cursor, `FXIconList`'s lasso rectangle) using the same
   four-filled-bands technique, via a general formula verified
   pixel-identical to the original `drawRectangle` output at scale=1:
   a stroked `drawRectangle(X,Y,W,H)` traces the same pixels as bands at
   `(X,Y,W+1,1)`/`(X,Y+H,W+1,1)`/`(X,Y,1,H+1)`/`(X+W,Y,1,H+1)`.
   Deliberately left alone: `FXDragCorner`'s resize-preview rectangle and
   `FXMDIChild`'s drag-rubberband box -- both XOR-drawn (`BLT_SRC_XOR_DST`)
   interactive overlays, conventionally thin regardless of scale, and
   `FXMDIChild` already correctly uses a scaled `setLineWidth()` with a
   matching inset.

**Nothing left in progress.** Everything above is committed and verified
by direct interaction (screenshots), not just compiled. Next up is still
Phase 4 polish (popups, drag corners, multi-monitor, GL canvases) --
genuinely not started. Minor loose ends, none blocking: `BitmapFontDialog`
still exists as ControlPanel's own dedicated shortcut alongside the now-
merged `FXFontDialog` (deliberate, not redundant -- see `b269a8a`'s
commit message); a bitmap family sharing its exact name with an installed
Xft family would collapse into that Xft entry in the merged list (edge
case, not hit in testing, noted in code).

---

## Status (2026-09-03, later)

Follow-up session, four things the user raised after playing with the
`73ddd30` ControlPanel build:

1. **`.FON` parsing is on the fly, not cached** -- confirmed by reading
   `fntLoad()`: it re-reads and re-parses the whole file from disk every
   `FXFont::create()`. No cache at any level. Fine for ControlPanel's
   usage; worth knowing if a `.fon`-backed font ever lands in a hot path.

2. **Real bitmap font picker, `19e5d30`.** Original plan was to fold
   bitmap fonts into the shared `FXFontSelector` widget so every FOX app
   benefits -- started down that path (see `72666e0`'s message for the
   detour), but the user redirected: too invasive for the win, keep it
   ControlPanel-only. Landed as:
   - `fxListBitmapFonts()` (`72666e0`, `lib/xfntface.h` + `FXFont.cpp`):
     scans a `PATHLISTSEP`-separated search path for `*.fon` files and
     extracts family/weight/italic/points from *every* embedded FNT
     resource (not just the closest-size match `fntLoad()` picks) --
     cheap, no glyph data touched. Real bug hit and fixed: `FXBitmapFontEntry`
     holds `FXString`s, so the usual `allocElms`/`resizeElms` (POD-only,
     no ctor/dtor) segfaulted -- switched to `FXArray<T>`, which does
     construct/destruct elements.
   - `controlpanel/BitmapFontDialog.{h,cpp}` (`19e5d30`): a small
     `FXDialogBox` with Family/Style/Size list columns and a live preview
     label, in the same spirit as `FXFontDialog` but for `.fon` files.
     Resolves the selection to a `"path,deci-points"` spec string, which
     turns out to already round-trip through `FXFont::setFont()`
     unchanged (it truncates `wantedName` at the first comma *before* the
     `.fon`-suffix check) -- no FXFont-side changes needed for this part.
     `FXBitmapFontEntry`/`fxListBitmapFonts()` are re-declared locally in
     the header rather than sharing `lib/xfntface.h` (library-internal,
     not installed) -- works because linking only cares about the
     mangled (namespace-qualified) name, not the struct's actual
     definition site; the same trick a throwaway test harness used first
     to validate the scan function stand-alone.
   - Search path is a compile-time default for now
     (`/usr/local/share/fonts:~/.local/share/fonts`,
     `BitmapFontDialog.cpp`'s `BITMAPFONTPATH`) -- **not yet** a registry
     setting/UI field. Cheap follow-up if this dialog earns its keep:
     mirror the existing "Icon Search Path" field exactly.
   - Verified end-to-end against two real downloaded fonts (MS Sans
     Serif/6 sizes, Fixedsys/1 size): family→style→size cascade, live
     preview, round-trip into `setupFont()`, Xft-controls greyout all
     confirmed via screenshots.
   - **Not done, explicitly deferred**: folding bitmap-font listing into
     the shared `FXFontSelector` widget itself (the original plan) --
     would need `FXFontDesc` (or a parallel path) to carry a bitmap
     font's file path/resource choice through `FXFontSelector`'s
     family/weight/style/size lists and `previewFont()`, touching code
     every FOX app depends on. Worth reconsidering only if
     `BitmapFontDialog` proves genuinely useful and the ROI justifies
     the shared-widget risk.

3. **Font-selector field semantics, and multi-size/style `.fon`
   support** -- explained to the user (Character Set/Set Width/Pitch/
   Scalable/All Fonts are Xft/fontconfig-only concepts, don't apply to
   `.fon`), and this turned out to already be the crux of item 2 above:
   `fntLoad()` only ever loads *one* resource (closest size); a `.fon`
   can bundle several sizes (confirmed: `sserife.fon` here has 6), and
   Windows ships separate *files* per weight/style rather than bundling
   those (this test file only has Regular). `fxListBitmapFonts()` /
   `BitmapFontDialog` are what now expose the full set to the user.

4. **Two real scale>1 rendering bugs found and fixed, `455c61c`**:
   - `FXWindow::scroll()`'s `XCopyArea` blit used its caller's *logical*
     coordinates directly against the *physical*-sized window -- correct
     only at scale=1. Visible as artifacts scrolling a long dropdown
     list (e.g. the Theme combo). The `addRepaint()` calls in the same
     function were already correctly logical (that contract dates to
     Phase 1/2's expose-event handling) and needed no change -- only the
     raw `XCopyArea` call needed `*scale`.
   - `FXFrame::drawBorderRectangle()` / `FXToolTip::onPaint()`: `FRAME_LINE`
     borders and the real tooltip popup border were both a single
     stroked `dc.drawRectangle()` -- an X11 "hairline", always exactly 1
     *physical* pixel regardless of scale (unlike the filled-band borders
     every sibling frame style already uses). Tried `dc.setLineWidth()`
     first -- not enough, since X11 centers a stroke on its path, so half
     of a scaled-width stroke drawn at a widget's own edge falls outside
     its bounds and gets clipped by whatever's next to it. Fixed by
     building the border from four filled bands instead, matching the
     existing sibling styles. Confirmed via pixel-level screenshot
     diffing: exactly `N*scale` physical pixels, fully contained, at both
     scale=1 (no regression) and scale=2.

---

## Status (2026-09-03, latest)

**Done: ControlPanel (FOX Desktop Setup) integration** (`73ddd30`). Built
on top of Phases 1-3 (native `.FON` bitmap fonts, registry scale setting,
`FXFont::isBitmapFont()` -- `b923980`, `59348b6`, `a6c341f`). Adds, per
the user's request:

1. A "UI Scaling" spinner (range 1-4) in the **General** tab, bound via
   `FXDataTarget target_scale` to `SETTINGS/scale` in the registry --
   read/written the same way as `barSize`/`maxcolors`/etc. `FXApp` already
   reads this key at startup (`b923980`), so the effect is system-wide the
   next time an app launches.
2. A "Bitmap Font..." button in the **Themes** tab next to the existing
   Xft "Choose Font..." button. Opens `FXFileDialog` filtered to
   `*.fon;*.FON`; on accept, sets `fontspec` to the picked path and calls
   the existing `setupFont()` -- no special-casing needed there, since
   `new FXFont(getApp(),fontspec)` already routes a comma-less `.fon` path
   through the native parser (`59348b6`).
3. The five Xft-only controls (hint-style/sub-pixel listboxes, hinting/
   autohint/antialias checkboxes) are now member pointers
   (`xftHintStyleList`, `xftSubpixelList`, `xftHintingCheck`,
   `xftAutohintCheck`, `xftAntialiasCheck`) instead of locals. A new
   `FXDesktopSetup::updateFontControlsEnabled()`, called at the end of
   `setupFont()`, disables all five when `font->isBitmapFont()` is true
   and re-enables them otherwise.

**Visually verified** (screenshots, via a small X11/XTest click+keysend
harness built ad hoc for this session -- not part of the repo): Themes tab
shows the new button; General tab shows the scale spinner; picking
`sserife.fon` (a real MS Sans Serif bitmap font, see below) through the
new file dialog greys out the Xft hint-style/sub-pixel listboxes
immediately with no crash; picking an Xft font again re-enables them.
Not separately verified: `SETTINGS/scale` actually persisting across a
ControlPanel restart (the write/read code follows the exact pattern of
every other setting in this file, so this is low-risk, but worth a quick
manual check next session if it matters).

**What's left for Phase 4 (polish, not started)**: see the original
Phase 4 notes further down this file. Nothing else is currently in
progress -- pick up Phase 4 fresh next session.

---

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
  - **Array/batch primitives (commit `d900aac`)**: `drawPoints`/`drawPointsRel`,
    `drawLines`/`drawLinesRel`, `drawLineSegments`, `drawRectangles`,
    `drawArcs`, `fillRectangles`, `fillChords`, `fillArcs`, and the six
    `fillPolygon*` variants now scale their caller-supplied arrays via new
    `scaledPoints`/`scaledRects`/`scaledArcs`/`scaledSegments` helpers
    (allocated with `allocElms`, freed by the caller after the X11 call).
    `FXArc`'s angle fields are left alone (already 1/64-degree units, not
    pixels). This closes out item 1 above — `FXDCWindow`'s X11 drawing
    surface is now fully scale-aware except for icon/image pixel content.
  - **`FXWindow::reparent` checked, needs no change**: `XReparentWindow` is
    always called at `(0,0)` relative to the new parent; actual position is
    established by a separate, already-scaled `position()`/`move()` call.
    Item 3 above turned out to be a non-issue.
  - **Systematic input/event translation (commit `e80d513`)**: KeyPress/
    KeyRelease and EnterNotify/LeaveNotify now unscale like Motion/Button
    events already did. More significantly, found that
    `FXWindow::translateCoordinatesFrom/To` — used on *every* mouse
    motion/button event while a grab is active (dragging a scrollbar,
    slider, splitter, ...) — go through a real `XTranslateCoordinates` call
    on physical X windows, not pure logical arithmetic as assumed; fixed,
    along with `getCursorPosition`/`setCursorPosition`
    (`XQueryPointer`/`XWarpPointer`) and `FXApp::findWindowAt`. XDND is a
    wire protocol shared with other, possibly unscaled, applications: the
    position/rectangle sent in `XdndPosition`/`XdndStatus` (both send and
    receive sides, in `FXWindow::handleDrag` and
    `FXApp::dispatchEvent`) now convert between real screen pixels on the
    wire and our internal logical convention. This closes out item 4 above.
  - **Tested beyond Pathfinder (this round)**: `tests/iconlist` (scrolled
    list — exercises the clip-rect fix directly), `tests/table` (grid lines,
    stipple hatch fill, cell-selection border, spanning cells — exercises
    most of the scalar/array primitive work), `tests/dialog` (buttons,
    separators), `tests/tabbook` (tabs, borders). All render correctly at
    scale=2; `tests/table` also verified at scale=3. This closes out item 5
    above (broad-enough coverage for now; still worth trying more apps as
    they come up).
  - **Icon/image pixel scaling (commit `665f409`)**: `FXImage`/`FXIcon`
    pixmaps are now physically NxN pixel-doubled at scale>1 while
    `width`/`height` stay logical. `FXImage::create()`/`resize()` scale the
    `XCreatePixmap` size; `create()` temporarily swaps `data`/`width`/`height`
    to a scaled buffer (via new `scalePixelsUp()`) right around the
    `render()` call, so all ~20 existing format-specific renderers and XShm
    sizing need no changes. `FXIcon::render()` turned out to compute its
    `shape`/`etch` masks directly from the same `data`/`width`/`height` as
    the color pixmap (not via a separate `FXBitmap`), so one swap keeps all
    three pixel-aligned — no `FXBitmap` changes needed, which meaningfully
    de-risked this compared to the investigation two updates ago.
    `FXDCWindow::drawImage`/`drawIcon*` updated to copy at the now-larger
    physical size. `drawBitmap` intentionally untouched (`FXBitmap`/stipple
    patterns conventionally stay native-resolution). This closes out item 4
    — the last big visible gap in Phase 2.
    - **Known gap, not fixed**: `FXImage::restore()` (reads pixels back
      from the pixmap into `data[]`) still uses unscaled `width`/`height`
      against the now-larger pixmap, so it would read the wrong region.
      Confirmed it's never called internally by the toolkit — opt-in
      application API only (e.g. a screen-grab tool) — so ordinary
      icon/image load-and-draw is unaffected. Fix it the same way as
      `create()` if/when something needs it.
    - Verified on Pathfinder (toolbar/tree/file-list icons visibly
      pixel-doubled, correctly positioned) and `tests/iconlist` (large icon
      view) at scale=2.
  - **Glyph-size scaling (commit `36f9c26`) — PHASE 2 DONE.** `FXFont` now
    has a second XftFont member, `displayFont`, opened at the physically-
    scaled pixel size and used only by `FXDCWindow::drawText`/
    `drawImageText` for rendering. `font` itself — and every metric method
    built on it (`getFontHeight`, `getTextWidth`, ...) — stays at the
    logical size, so layout math is never lied to. This is exactly the fix
    the Phase 1 revert (above) said was missing.
    - `match()` mutates `xid` and the `actual*` metadata fields as a side
      effect of matching a font; creating `displayFont` via a second
      `match()` call saves/restores those around it so they keep
      describing `font` (the logical one) exactly as before, and restores
      `xid=(FXID)font` afterward.
    - At scale=1, `displayFont` is just set to `font` — no second FcMatch,
      no extra X server resource, and (verified) rendering is bit-identical
      to before this commit.
    - `drawImageText`'s erase-rectangle position/size was previously
      entirely unscaled (a pre-existing gap, unrelated to font size) — now
      scales too, fixed incidentally while touching this code.
    - Verified on Pathfinder and `tests/table` at scale=2: text renders at
      the correct bigger size, correctly positioned, full layout intact —
      no double-scaling. `tests/table` at scale=1 confirmed unchanged.

  **Phase 2 is now done.** Every item from the original plan (geometry,
  drawing primitives — scalar and array/batch, icon/image pixel content,
  clipping, systematic input/event translation, glyph size) is implemented
  and verified across Pathfinder plus five `tests/*` example apps at
  scale=2 (table also at scale=3). Remaining known gaps are narrow and
  deliberately deferred, not blockers: `FXImage::restore()` (opt-in API,
  no internal callers), `FXWindow` popups/drag-corners/multi-monitor/GL
  canvases (Phase 4 polish, per the original plan).

- **Phase 3 (commit `888b1d0`): native bitmap font support, landed.**
  `FXFont` gets a `bitmapFont` member (a genuine `XFontStruct*`, loaded
  via classic `XLoadQueryFont`) alongside the normal Xft `font`/
  `displayFont`, opt-in per font by name — `new FXFont(app,"9x15")` (the
  existing single-string constructor already routes any comma-less name to
  `hints=FXFont::X11`, so no new API was needed). Metrics
  (`hasChar`/`getFontWidth`/`Height`/`Ascent`/`Descent`/`getCharWidth`/
  `getTextWidth`/`getTextHeight`) branch to real `XFontStruct` logic
  (copied from the existing but dormant-when-Xft-is-on XLFD backend) when
  `bitmapFont` is set, so bitmap-font widgets lay out using the bitmap
  font's real metrics. `FXDCWindow::drawBitmapText()` renders glyphs as
  pixel-perfect NxN blocks: draws at native 1x onto an offscreen 1-bit
  pixmap via classic `XDrawString16`, nearest-neighbor duplicates pixels
  into NxN blocks for scale>1 (same technique as
  `FXImage::scalePixelsUp()`), then uses the (scaled) bitmap as a clip
  mask and fills through it with the foreground color — transparent
  background, like `drawIcon()`'s mask, not an opaque box.
  - Validated the core rendering technique with a standalone POC against
    real PCF bitmap fonts already on this system (`/usr/share/fonts/X11/misc`,
    e.g. `9x15`) *before* touching `FXFont`/`FXDCWindow` — this Linux
    system doesn't need a `.FON` file to test against; the classic X11
    core-font path was exactly the right vehicle.
  - Hit two real, non-obvious rendering bugs while integrating (see the
    commit message for full detail): (1) `XCopyPlane` paints an opaque
    box (both fg and bg), and `FXDCWindow`'s default `devbg=0` happens to
    be black too — came out as a solid black rectangle; switched to a
    clip-mask + `fillRectangle` approach for transparency. (2) That clip
    mask worked at scale=1 (drawn directly by the server) but came out
    *inverted* at scale>1 (rebuilt client-side via `XGetImage`/
    `XPutImage`): `XPutImage` with `XYBitmap` format treats the image as
    a stencil using the GC's fg/bg pixels, and X11's default GC has
    foreground=0/background=1 — exactly inverting the bit pattern on
    upload. Fixed by setting foreground=1/background=0 on the upload GC.
  - Verified: a minimal custom FOX app renders `9x15` bitmap-font text
    correctly (transparent background, crisp NxN blocks confirmed by
    pixel-level crop) across `FXLabel`, `FXButton`, and `FXTextField`,
    alongside a normal Xft label in the same window, at scale=1, 2, 3.
    Pathfinder at scale=2 (no bitmap font involved) confirmed unchanged.
  - **Not done**: React95/`.FON`-specific testing (this used the system's
    real PCF fonts instead, which satisfies the same "genuine bitmap
    font, not a smoothed derivative" requirement); `FXText`/multi-line
    widgets not specifically tested; array-based bitmap-font drawing
    (`drawLine` etc. with a bitmap font active is irrelevant — only text
    rendering is font-specific).

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
