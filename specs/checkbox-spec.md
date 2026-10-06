# Checkbox control: brief for the Win95/98/2000 controls project

Evidence tags used below:
- **[measured]** read from real, uncropped, lossless screenshots
- **[wine]** from Wine's `button.c` (a reimplementation: corroborating, not ground truth)
- **[assumed]** my best guess of classic Win95 behaviour, NOT yet checked against a capture
- **[hypothesis]** a proposed explanation to test
- **[replica]** observed on the FOX replica (`CYTCheckBox`) rendering this brief; shows the brief is self-consistent and implementable, not that real Windows does it

## Changes in this revision
- **CORRECTED, tick pixel count:** the brief said the tick has "19 pixel positions" (twice), but its own coordinate list has **21**: row 3: 1, row 4: 2, row 5: 4, row 6: 5, row 7: 5, row 8: 3, row 9: 1. The coordinates are kept. If 19 was the count actually checked against the captures, the list and the check disagree, and the captures need re-checking.
- **CORRECTED, box table legend:** the drawn table used F (face) for the inner bottom-right ring and W (highlight) for the interior. Both are wrong roles: the inner bottom-right ring is 3D light (#DFDFDF on Win98, not the face), and the interior is the window color (it turns to face when disabled while the W ring does not). The table now uses separate letters L and i, matching the region table.
- **CORRECTED, pressed state:** the tick while held is the state from **before** the press; the check toggles only on release. After release the box is drawn normally (window interior) with the new state, and **the focus rectangle stays**. [wine] A replica that toggles at press and redraws only on a state change leaves the pressed look on screen after release. [replica]
- **CORRECTED, contradiction in the vertical-offset hypothesis:** centering the 13px box with (h - 13) / 2 puts it at row 1 of a 16px control, which contradicts the measured rect-flush layout (box at row 2). The box term must be (h - 12) / 2. Also, the contact sheet's flipped-phase tile (17px control, box at row 3, text at box-1) does not follow this hypothesis, which puts a 17px control's text level with the box (box+0, as the disabled capture measured). See "Open question".
- **ADDED, minimum control width:** 19 + text width, so the focus rectangle fits.
- **ADDED, third state (MAYBE):** an assumed rendering, so a replica has something defined.
- **ADDED, label origin check:** the first ink column is not the text origin. Verify with the underline or a glyph whose first column is inked.
- **ADDED, color derivation and replica results** (last two sections).

## Goal and method

Pixel-exact replica of classic (non-themed) Windows 95/98/2000 controls. Everything is verified by pixel-diffing renders against real, uncropped, lossless screenshots. A control's bounding box never changes; decoration is drawn inside it, and later layers overwrite earlier ones.

Text uses the real MS Sans Serif 8pt bitmap font (`sserife.fon`): lineHeight 13, ascent 11, internalLeading 2. Label width is the sum of the glyph advance widths.

### Font file facts (`sserife.fon`)
- NE container with six FNT v2.00 fonts (8, 10, 12, 14, 18, 24 pt, all 96 dpi). The 8pt font is the first, at file offset 1200, length 4048. It has height 13, ascent 11, chars 32..255. Per-char table: `(width u16, offset u16)`, offsets absolute within the FNT.
- Glyphs are stored in `ceil(width/8)`-wide bands: for each band, `lineHeight` bytes, one per row, MSB = leftmost pixel. This is indistinguishable from plain row-major for glyphs of 8px or less, so verify any reimplementation on a wider glyph such as "M" (9px).
- Codes 128..159 (except the euro sign at 128) render as the font's default glyph, a solid vertical bar. Do not treat that as a decoding bug.
- Many glyphs have a blank first column (e.g. T, S, D, I). When locating a label's text origin, use the mnemonic underline (it starts exactly at the glyph's advance origin) or a glyph inked in its first column, not the first ink column of an arbitrary label. [replica: "Disconnect" and "Int 13 unit" ink starts at +19 with the origin at +18]

## Checkbox facts

### Box: 13x13, local (col,row) from the box's top-left pixel [measured, Win95, 640x480, 3 checkboxes in one dialog]
Legend: G outer top-left (3D shadow, #808080) · K inner top-left (3D dark shadow, #000000 on Win95) · L inner bottom-right (3D light, #C0C0C0 on Win95) · W outer bottom-right (3D highlight, #FFFFFF) · i interior (window, #FFFFFF when enabled)
```
row 0     GGGGGGGGGGGGW
row 1     GKKKKKKKKKKLW
rows 2-10 GKiiiiiiiiiLW   (9 rows; interior is a 9x9 square at cols 2..10)
row 11    GLLLLLLLLLLLW
row 12    WWWWWWWWWWWWW
```
On Win95, L equals the face and i equals W, which is why an earlier version of this table drew them as F and W. The roles differ: L becomes #DFDFDF on Win98, and i becomes the face when disabled.

Checked: black (K) tick pixels, **21 in total**, local (col,row): row 3: 9 | row 4: 8,9 | row 5: 3,7,8,9 | row 6: 3,4,6,7,8 | row 7: 3,4,5,6,7 | row 8: 4,5,6 | row 9: 5

[wine] Wine sizes the check box as `12 * dpi/96 + 1`, which is 13 at 96 dpi. This matches the measured 13x13.

### Box across Windows 95, 98 and 2000 [measured: `Windows95_Checkboxes.png` 640x480, `Windows98_Checkboxes.png` 1023x768, `Windows2000_Checkboxes.png` 1280x720]
Compared on every checkbox in the captures: 5 in Win95 (1 enabled unchecked, 3 disabled unchecked, 1 disabled checked; the same 5 appear in `Disabled_Checkboxes.png`), 3 in Win98 (1 unchecked, 2 checked, all enabled), and 5 in Win2000 (4 enabled checked, 1 disabled checked). An exhaustive scan of each capture for exact matches of this layout found no other boxes. All 13 boxes match the table with 0 differing pixels (the box, not the label). Every box is 13x13 with the same pixel layout as the table above, and the checked tick uses the same pixel positions in all three (verified: the non-interior pixels equal the tick set exactly; the set is the 21 listed above — see the count correction at the top). Each region below is a single uniform color within every box.

| Region (local pixels) | Win95 | Win98 | Win2000 | System color role |
|---|---|---|---|---|
| Outer top-left: row 0 cols 0-11, col 0 rows 1-11 | #808080 | #808080 | #808080 | 3D shadow |
| Inner top-left: row 1 cols 1-10, col 1 rows 2-10 | #000000 | #000000 | #404040 | 3D dark shadow |
| Inner bottom-right: col 11 rows 1-11, row 11 cols 1-10 | #C0C0C0 | #DFDFDF | #D4D0C8 | 3D light |
| Outer bottom-right: col 12 rows 0-12, row 12 cols 0-11 | #FFFFFF | #FFFFFF | #FFFFFF | 3D highlight |
| Interior, enabled (cols 2-10, rows 2-10) | #FFFFFF | #FFFFFF | #FFFFFF | window |
| Interior, disabled | #C0C0C0 | not captured | #D4D0C8 | 3D face |
| Tick, enabled | #000000 (not captured; black per brief) | #000000 | #000000 | window text |
| Tick, disabled | #808080 | not captured | #808080 | 3D shadow |
| Dialog face (background) | #C0C0C0 | #C0C0C0 | #D4D0C8 | 3D face |

- **Structure and geometry are identical in all three.** Only colors differ, and each color follows a system color role.
- **It is not true that only the background (face) differs.** Two regions also change: the Win98 inner bottom-right ring is #DFDFDF, which is neither its face (#C0C0C0) nor Win95's ring; and the Win2000 inner top-left edge is #404040, not #000000. In Win95 and Win2000 the inner bottom-right ring equals the face color; in Win98 it does not.
- Win98 disabled box and tick were not in the captures, so the disabled colors for Win98 are unmeasured. By the table's role mapping they would be face interior and #808080 tick.

### Deriving the version colors from a palette
Per-channel average, rounded down, from the basic palette (face, highlight, shadow, frame):
- 3D dark shadow, Win2000 only: (frame + shadow) / 2 = **#404040**. Win95/98 use the frame color (#000000).
- 3D light, Win98 only: (face + highlight) / 2 = **#DFDFDF**. Win95/2000 use the face. (On Win2000 the formula would give #E9E7E3, not the measured #D4D0C8.)
Same formulas as the push button and radio button.

### Text origin by capture [measured by matching the real font against the labels]
- Win95 capture: text origin = box+(18,0) (5 labels, 0 mismatches). The labels shown are not underlined.
- Win98 capture: text origin = box+(18,-1) (3 labels, 0 mismatches; text widths 151, 196, 163).
- Win2000 capture: the label font does not match `sserife.fon` (hundreds of mismatched pixels), so no text origin could be derived from it.
- The three text widths in this Win98 capture (151, 196, 163) are exactly the widths quoted in the focus-rectangle section's evidence, and its text origin is the (18,-1) that section uses. Those three captures may therefore be the Win98 dialog and not Win95. The Win95 capture here shows (18,0), so the -1/-2 offset may depend on the OS version rather than on control height. This is a hypothesis; the focus rectangles themselves were not measured in these three files.

### Label and its focus rectangle [measured]
- Text origin (top-left of the 13-row glyph cell) = (boxLeft+18, boxTop-1) in the Win98 captures; (boxLeft+18, boxTop+0) in the Win95 ones. The vertical offset is not constant; see "Open question".
- Focus rectangle (inclusive): x0=tx-1, y0=ty-1, x1=tx+textWidth, y1=ty+14. So width = textWidth+2 and height = 16. Matches 3/3 captures (text widths 151, 196, 163 give rect widths 153, 198, 165).
- Rect left = boxLeft+17. Rect top = boxTop-2 in all three Win95 captures. Two Win98 shots from different dialogs showed -1 and -2, so the vertical offset is not constant.
- Mnemonic: the `&` is not drawn. The next character is underlined in the bottom row of its glyph cell (row 12 = ascent + 1), across its advance width (seen on Browse and Apply).
  - [measured] Win98 capture, 3 checkbox labels: the underline is exactly row 12 over the full advance of the mnemonic character, on the E of "Energy", the P of "Plug" and the R of "Reset" (0 pixel mismatches including the underline).
  - [measured] Win95 capture: none of the 5 labels is underlined. It is not known whether those labels contain an `&`, so this does not confirm that Win95 shows underlines by default.
  - [measured] Win2000 capture: none of the 4 enabled labels is underlined; the disabled one was not scanned.

### Control rectangle and clickable area
- The control rectangle is the whole window rect, and it is the clickable area. A press counts if the point is inside it, and a release activates only if it is inside it too. [wine: hit tests use `PtInRect(GetClientRect)` on WM_LBUTTONUP and WM_MOUSEMOVE]
- There is no margin around the box or label. [wine] In the Win95 captures the origin is (boxLeft, boxTop-2): the rect's top edge touches the control's top, and the left edge of the box touches the control's left. [measured, via the focus-rect offsets]
- **Minimum width:** 19 + textWidth. The focus rectangle's right edge is at local x = 18 + textWidth, so a narrower control clips it. [follows from the measured offsets]
- The control may be wider than box + label. Nothing extra is drawn in that case: the box stays 13x13 and the focus rectangle still wraps only the text (textWidth+2 x 16). Only the clickable area is wider.
- Control height is not yet measured. The contact sheet assumes 16 (flush with the focus rectangle). [assumed]
- While the button is held, the pressed look tracks the pointer: on when inside the control rect, off when outside, back on if it returns before release. [wine, `WM_MOUSEMOVE` while captured]

### Right-to-left layout [assumed: no RTL capture exists yet]
Scope: Latin labels only. `sserife.fon` has no Arabic or Hebrew glyphs, and a single Latin run keeps its left-to-right glyph order under RTL reading order, so only the layout changes. Test labels should not end in punctuation: under RTL reading order, trailing punctuation can move to the visual left of the text.
- **Model [assumed]:** the LTR layout mirrored about the control's vertical axis, with the glyph pixels themselves NOT mirrored. With control width W (local x, 0 = control's left edge):
  - the 13x13 box (same pixels and colors as LTR) is flush with the right edge: local x = W-13..W-1
  - the label's text starts at local x = W-18-textWidth, so its last column is W-19 and there are 5 columns between text and box (LTR: 5 columns between box and text, cols 13..17)
  - vertical rules are unchanged: text top = boxTop + text_dy, box rows and mnemonic row (row 12 of the glyph cell) as in LTR
  - focus rectangle (inclusive): x0 = textStart-1 = W-19-textWidth, x1 = textStart+textWidth = W-18, y as in LTR. With the default control width it is flush with the control's left edge, and 4 columns separate it from the box (LTR: 4 columns between box and rect)
  - hit area = the whole control rectangle; a widened control grows to the LEFT, and the box stays on the right edge
- **Dot phase [assumed]:** same rule in local coordinates measured from the control's visual top-left (dot where local `x+y` is odd). Whether a real mirrored control anchors the checkerboard to its left or right edge is unknown, so the sheet shows both phases (origin dy=-2 and the flipped dy=-3 variant), like LTR.
- **Evidence [wine]:**
  - LTR agrees with the measurement: Wine puts the label rect at box width + 4 = 17 and then shifts it 1px right, giving 18.
  - With `WS_EX_RIGHT`, Wine right-aligns the label (`DT_RIGHT`) and places its right edge 1px inside the control (`r.right = rc->right - 1`, the mirror of the `r.left++` used on the left). This agrees with the model.
  - I did not see Wine's code that places the box on the right (the fetched source was cut off before it), so the 13+4 gap on the right side is the mirror assumption.
  - Per the Win32 documentation, `BS_LEFTTEXT` (same as `BS_RIGHTBUTTON`) also puts the box on the right without any RTL flag; the same layout would apply, unmeasured.
- **What real captures must settle** (Win95/98/2000, with `WS_EX_RIGHT` and, where available, `WS_EX_LAYOUTRTL`):
  1. Are the box pixels mirrored under layout mirroring (shadow/highlight sides flip), or unchanged?
  2. The box's offset from the control's right edge, and the text-to-box gap.
  3. The focus-rectangle dot phase.
  4. The mnemonic underline position and the tick orientation.
  5. Any difference between `WS_EX_RIGHT` only and `WS_EX_LAYOUTRTL`.

### Disabled and clicked states
- **Disabled (engraved variant) [contradicted by the measured capture below; kept for reference]**: box interior is #C0C0C0 instead of white. Border rows/cols unchanged. If checked, tick is #808080 (same pixels). Label is engraved: draw the text in #FFFFFF at (+1,+1), then in #808080 at (0,0). The mnemonic underline is engraved the same way. No focus rectangle.
- **Disabled, flat label [measured, `Disabled_Checkboxes.png`, 640x480, lossless, 5 checkboxes in one dialog: 3 disabled-unchecked, 1 disabled-checked, 1 enabled-unchecked for comparison]**
  - Box: same 13x13 as the enabled box except the 9x9 interior is #C0C0C0 (face) instead of white. Border pixels are identical to the enabled box (rows 0, 1, 11, 12 and cols 0, 1, 11, 12 as in the table above).
  - Tick, if checked: exactly the same pixel positions as the black tick (the 21 listed above), but #808080.
  - Label: #808080 only, drawn once. There is NO white offset copy, so the label is not engraved. Verified by matching the real font against the capture: "Disconnect", "Sync data transfer", "Auto insert notification", "Int 13 unit" (disabled, gray) and "Removable" (enabled, black) all matched with 0 pixel mismatches.
  - Text origin in this capture is boxLeft+18, boxTop+0. The brief's three Win95 captures had boxTop-1, so the text sits 1px lower relative to the box here. This fits the known variation in the focus-rect offset (a rect top of boxTop-1 instead of boxTop-2), and may be the same unexplained cause as the -1/-2 case. No focus rectangle is visible on disabled controls, so the control rectangle cannot be measured from this capture.
  - Mnemonic: none of the five labels shows an underline (including the enabled "Removable"). Either the labels have no `&`, or the dialog hides the underline until Alt. Not resolved by this capture.
  - Box tops in the dialog are 23px apart (199, 222, 245). The two columns have their box left edges at x=78 and x=240.
  - Lone cross-check: the enabled "Removable" box matches the measured enabled box table exactly (white 9x9 interior, #C0C0C0 in col 11).
- **Clicked (pressed) [CORRECTED; pixels assumed, sequence from wine]:**
  - Mouse down: the checkbox takes focus and the capture and shows the pressed look: interior #C0C0C0 (face), as the disabled box. **The check state does not change**: the tick shows the state from before the press (black if it was checked, absent if not). Label drawn normally. The focus rectangle is present (pressing takes focus). [wine: WM_LBUTTONDOWN does `SetCapture`, `SetFocus`, `BM_SETSTATE TRUE`]
  - While held, the pressed look follows the pointer (see "Control rectangle and clickable area"). The tick does not change while held.
  - Release inside the control rectangle: the state toggles, then BN_CLICKED. Release outside: no change. [wine: WM_LBUTTONUP, `PtInRect` → BM_SETCHECK]
  - **After release:** the box returns to the normal look (window interior) showing the new state, and **the focus rectangle stays**: nothing takes focus away.
  - Space bar: the same sequence, with key down as press and key up as release. [wine: WM_KEYDOWN/WM_KEYUP VK_SPACE]
- Open question: whether the real pressed interior is solid #C0C0C0 or a dither of #C0C0C0/#FFFFFF.
- **Third state (BS_3STATE, indeterminate) [assumed, no capture]:** interior #C0C0C0 (face) and the tick in #808080, the same as a disabled checked box but with a normal label and focus rectangle. Unverified; a capture should settle whether the interior is solid face or dithered.

## Focus rectangle behaviour (must be applied here) [measured]

- The dots are a fixed checkerboard in the control's own coordinates (`(0,0)` = the control's top-left pixel): draw `SHADOW_DARK` where local `x+y` is ODD and skip the pixel where it is even. The rectangle's corner does not set the phase; the parity of its local position does.
- It is an XOR with a checkerboard brush. Black on a FACE background; on other backgrounds the dots take other colors (e.g. yellow on navy). A replica that draws plain black dots is only correct on the face color.
- A checkbox does NOT always start transparent. That is only true for command buttons, whose rectangle sits at local (4,4), an even sum. For a checkbox, the render must take the control's origin parity (or its height) as an input. Do not hard-code the start.
  - [measured] Command-button rule checked on the Win98 "Change..." default push button (75x23 control): focus rectangle at local (4,4)..(70,18) (67x15), corner sum even so it starts transparent; all 160 perimeter pixels satisfy dot <=> local `x+y` odd (0 mismatches). One button in one capture only.
- Evidence the anchor is the control itself:
  - Dragging a window by 1px never changes the start, so it is not screen-anchored.
  - In one dialog, three checkboxes were 24px then 25px apart. All three started transparent relative to their own corner, while the third's absolute parity flipped. A dialog anchor would have kept the same absolute parity.
  - OK/Cancel/Apply all start transparent despite origins of odd, even, odd parity.
  - Two desktop labels with identical rects were exact complements, so it is not per-item either.
  - Shifting a rectangle by 1px within its control flips its start; moving the whole control does not.
- Verify by checking every perimeter pixel: dot <=> local `x+y` odd.
- Reference render: with control origin = (boxLeft, boxTop-2) the rect's top-left corner is at local (17,0), an odd sum, so it starts with a dot. With origin = (boxLeft, boxTop-1) the corner is at (17,1), even, so it starts transparent. The two renders are exact complements. Flipping the phase needs the origin an odd number of pixels away from the rectangle's corner, so the control must have 1px more background above the rectangle (or to its left) than in the unflipped case; it cannot be flush. On the contact sheet the flipped variant uses origin (boxLeft, boxTop-3) with a 17px-tall control. The extra control row is drawn as the tile's top gray (that tile has no separate top margin), so both focus pairs look identical apart from the dot phase: 1px of gray above and below the rectangle, box at the same row. The 17px height is an assumption, not a measurement.
  - **Inconsistent with the vertical-offset hypothesis below:** under integer centering, a 17px control has its box at row 2 and its text at row 2 (text = box+0), so the rect corner is at (17,1), even, and it starts transparent. The flipped tile instead puts the box at row 3 with text = box-1. Either the hypothesis or the tile is wrong; a capture of a 17px checkbox with focus settles it.

## Open question: the -1/-2 vertical offset [hypothesis]
[wine] The label rectangle is vertically centered in the client rect with integer division (`top = rc.top + (rc.height - n) / 2`), then nudged by one pixel for the focus rectangle. If the real control does the same, the label's top, and with it the rect top, depends on the parity of the control height. That would explain -2 vs -1 across dialogs without any other cause. To test: record each captured checkbox's control height (from the dialog template or resource) and see whether -1 corresponds to one parity and -2 to the other. If it holds, control height is also the input that fixes the dot phase.
- **The box term must be (h - 12) / 2, not (h - 13) / 2 (CORRECTED).** Centering the 13px box gives row 1 in a 16px control, but the measured rect-flush layout has the box at row 2. With box top = (h - 12) / 2 and text top = (h - 13) / 2:
  - h=16: box row 2, text row 1 (text = box-1; rect top 0, flush; corner (17,0), dot) — matches the Win98 captures.
  - h=17: box row 2, text row 2 (text = box+0; rect top 1; corner (17,1), no dot) — matches the disabled capture's box+0.
  This is consistent with both measured offsets but does not explain the Win95 rect-top of boxTop-2 together with text box+0; that still needs control heights from the captures. [replica: these formulas reproduce box-1 at h=16 and box+0 at h=17]

## Not yet measured
- Win98 disabled box and tick colors. (Win98 inner ring #DFDFDF, Win2000 face #D4D0C8 and inner edge #404040 are now measured; see the cross-version table.)
- Pressed and 3-state checkbox pixels, against real captures (the rules above are assumptions; the press/release sequence is from wine); radio buttons. Disabled is now measured (flat label); the engraved variant is contradicted.
- Control height in the captures, and whether the pressed interior is dithered.
- Everything about the right-to-left layout (see the RTL section): no capture exists yet.

## Contact sheet
`checkbox_contact_sheet.png` is rendered 1:1: every pixel in the PNG is one screen pixel, and its captions use the same 8pt font. Each tile is the control rect plus a 1px #C0C0C0 margin on every side, except the flipped-phase tiles, which have no top margin because their 17px control already includes the extra gray row (the unflipped control is 16px). The red hit-area outline is drawn 1px outside the control rect so it never overwrites a control pixel. The two "flat label [measured]" tiles use the real labels ("Disconnect" unchecked, "Int 13 unit" checked), text origin box+(18,0), and an assumed 16px control that starts 1px above the text (rect-flush, same rule as the other tiles); they are pixel-identical to the capture across the control rect (73x16 and 66x16, 0 diffs). The ten RTL tiles (five pairs: plain, focus, flipped focus phase, flat disabled, clicked) and the two RTL widened tiles follow the RTL section, are all [assumed], and use Latin labels. Their box pixels are identical to the LTR box, the text-to-box gap is 5 columns, and the focus rectangle is flush with the control's left edge. Self-checks run on the saved PNG: box and tick pixels equal the tables above, each tile equals its in-memory render byte for byte, and every focus-rect perimeter pixel satisfies dot <=> local `x+y` odd.
- Note: the "flat label" tiles put a 16px control 1px above text that sits at box+0, i.e. box at row 1. That differs from the other tiles (box at row 2) and from the centering hypothesis (box+0 needs a 17px control). The control-rect placement of those tiles is assumed, not measured.

## Implementation notes from the replica [replica]
Rendered by `CYTCheckBox` (FOX, Win95 palette, `sserife.fon` 8pt), read back from the X server:
- 9/9 replica boxes match the box table exactly: enabled (white interior, black tick on the 21 listed pixels), disabled (face interior, gray tick), third state (face interior, gray tick, as assumed).
- Underlined labels start exactly at box+18; the underline row puts the text top at box-1 for h=16 and box+0 for h=17.
- Real (XTest) clicks: while held, face interior with the pre-press tick; after release, white interior with the toggled state; the focus rectangle persists after release. Two consecutive clicks round-trip.
- **Redraw when the press ends.** A toolkit that redraws only on a state change will leave the pressed look on screen after release: by release time the state is already final (FOX's `FXCheckButton` toggles at press).
- **Show the pre-press state while held.** A toolkit checkbox that toggles at press must draw its old state until release.
- Side observation: FOX's own stock `FXCheckButton` unchecked box is pixel-identical to the Win95 table; the differences are the tick shape, disabled colors and label placement.
