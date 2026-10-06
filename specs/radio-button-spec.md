# Radio button spec: Win95 / Win98 / Win2000 classic controls

Evidence tags: **[measured]** read from the supplied captures · **[wine]** Wine `button.c` · **[assumed]** best guess, not checked against a capture · **[hypothesis]** proposed explanation, to test · **[replica]** observed on the FOX replica (`CYTRadioButton`) rendering this spec; shows the spec is self-consistent and implementable, not that real Windows does it.
Everything below was produced by `radio_contact_sheet.py` (it re-measures the captures and prints the counts). Sheets: `radio_contact_sheet_win95.png`, `_win98.png`, `_win2000.png`.

## Changes in this revision
- **CORRECTED, pressed state (section 4):** "Pressed: interior = face, dot black" read as if the dot appears when you press. It does not. While held, the radio keeps its pre-press state: an unselected radio shows the pressed look with no dot, and the previously selected radio keeps its dot. The selection moves only on release, and the rest of the group is cleared in the same step. [wine] A replica that turns the dot on at press shows two dots during the press. [replica]
- **ADDED, release and focus (section 4):** release inside selects; release outside changes nothing; the pressed look follows the pointer; the focus rectangle stays after release. [wine]
- **ADDED, repaint requirement (section 7):** a replica must redraw when the press ends, not only when the selection changes, or the pressed look stays on screen after release. [replica]
- **ADDED, label origin check (section 3):** the first ink column is not the text origin. "Te&xtured" and "Soli&d" start their ink at +18 although the origin is +17: T and S have a blank first column. Verify the origin with the mnemonic underline, which starts exactly at the glyph's advance origin, or with a glyph whose first column is inked ("M"). [replica]
- **ADDED, disabled label default (section 4):** flat should be the default. The only measured disabled label of any classic control (the Win95 checkbox capture) is flat. Engraved stays available as an option.
- **ADDED, keyboard vs. mouse (section 5):** arrow-key navigation in a dialog selects a radio as soon as it gets focus; the mouse selects on release. [assumed]
- **ADDED, color derivation (section 2):** the version-specific colors follow from the face, highlight, shadow and frame colors.
- **ADDED, section 7:** results of rendering this spec.

## 0. Corrections to the brief and to the first sheet
- **CONTRADICTED:** "expect the same 13x13 glyph cell [wine]". The circle is **12x12**, not 13x13 or anything that fills a 13x13 cell. [measured, 10 instances]
- **CONTRADICTED:** "same label offset as the checkbox". The label starts **17px** right of the circle's left edge (checkbox: 18 from the box edge). [measured, Win95 x2, Win98 x6]
- **CONTRADICTED:** my first sheet's circle and dot were hand-drawn guesses (13x13, 5x5 diamond dot). Both were wrong and are replaced by the measured bitmaps below.

## 1. Provenance of the captures
| File | Size | What it is |
|---|---|---|
| Win95_RadioButton.png | 688x64 | **2x nearest-neighbour upscale**, grid offset (1,1), +-3 level noise (colors like #010101, #BEBEBE). Not 1:1 as requested. Downsampled at offset (1,1) and snapped to {#C0C0C0, #000000, #FFFFFF, #808080}; max deviation before snapping 3; every 2x2 block snapped within tolerance. 2 radios (1 checked, 1 unchecked). |
| Win98_RadioButton.png | 508x206 | 1:1. 6 radios (3 checked, 3 unchecked), 3D Pipes Setup. A few text pixels are #010101 instead of #000000 (noise); the circles are exact. |
| Win2000_RadioButton.png | 357x274 | 1:1. 2 radios (1 checked, 1 unchecked), Performance Options. Labels in this capture are Tahoma, not `sserife.fon` (other Win2000 screens do use `sserife.fon`). |

## 2. Circle bitmap [measured]
12x12, local (col,row), identical on Win95, Win98, Win2000 (all 10 instances match the table exactly: Win95 2/2, Win98 6/6, Win2000 2/2).
```
G outer top-left (3D shadow)        K inner top-left (3D dark shadow)
L inner bottom-right (3D light)     W outer bottom-right (3D highlight)
i interior                          . outside the circle (dialog face)
row 0   ....GGGG....
row 1   ..GGKKKKGG..
row 2   .GKKiiiiKKW.
row 3   .GKiiiiiiLW.
rows 4-7 GKiiiiiiiiLW
row 8   .GKiiiiiiLW.
row 9   .GLLiiiiLLW.
row 10  ..WWLLLLWW..
row 11  ....WWWW....
```
Checked dot, 4x4 with the corners off, black on all three: row 4 cols 5-6 | rows 5-6 cols 4-7 | row 7 cols 5-6.
Note the top-right is not a mirror of the bottom-left: row 2 has KK at cols 8-9 and a highlight W at col 10.

### Colors by region [measured]
| Region | Win95 | Win98 | Win2000 | Role |
|---|---|---|---|---|
| G outer top-left | #808080 | #808080 | #808080 | 3D shadow |
| K inner top-left | #000000 | #000000 | #404040 | 3D dark shadow |
| L inner bottom-right | #C0C0C0 | #DFDFDF | #D4D0C8 | 3D light |
| W outer bottom-right | #FFFFFF | #FFFFFF | #FFFFFF | 3D highlight |
| Interior, enabled | #FFFFFF | #FFFFFF | #FFFFFF | window |
| Dot, enabled | #000000 | #000000 | #000000 | window text |
| Dialog face | #C0C0C0 | #C0C0C0 | #D4D0C8 | 3D face |
On Win95 and Win2000 the L pixels equal the face, so the capture cannot tell "ring" from "background"; the L role is inferred from Win98, where they are #DFDFDF [measured on Win98, role carried over as assumed].

### Deriving the version colors from a palette
Each measured value follows from the basic palette (face, highlight, shadow, frame) with a per-channel average, rounded down:
- 3D dark shadow, Win2000 only: (frame + shadow) / 2 = (#000000 + #808080) / 2 = **#404040**. Win95/98 use the frame color (#000000).
- 3D light, Win98 only: (face + highlight) / 2 = (#C0C0C0 + #FFFFFF) / 2 = **#DFDFDF**. Win95/2000 use the face color. On Win2000 the formula would give #E9E7E3, not the measured #D4D0C8, so Win2000 really does use the face.
These are the same formulas the push button and checkbox use.

## 3. Label [measured unless marked]
- **x:** text origin = circle left + 17. Font matching with `sserife.fon`, underline included: Win95 2/2, Win98 6/6 at 0 mismatches (Win98 Traditional and Textured: 22 and 7 pixels that are #010101 instead of #000000; 0 mismatches when pixels <= 3 count as black).
  - **How to verify the origin:** use the mnemonic underline, which starts at the glyph's advance origin, or a glyph whose first column is inked ("M"). Do not use the first ink column of an arbitrary label: T and S have a blank first column, so "Te&xtured" and "Soli&d" show ink from +18 at a correct origin of +17. [replica]
- **y:** text top = circle top in Win95 (2/2) and Win98 Single, Multiple, Traditional, Flex; text top = **circle top - 1** in Win98 Solid and Textured (same dialog, different group boxes).
- **[hypothesis]** the 0 / -1 difference is control-height parity: with a 12px glyph and a 13px text line both centred by integer division, an even height (16) gives text = circle - 1 and an odd height (17) gives text = circle. Settled by one capture showing a control's bounds (hit-area test or two radios with known heights).
  - Formulas: circle top = (h - 12) / 2, text top = (h - 13) / 2, integer division. h=16 gives circle 2, text 1; h=17 gives circle 2, text 2. The replica uses exactly these and reproduces both measured offsets (underline row confirms text top = circle - 1 at h=16, = circle at h=17). [replica]
- **Mnemonic:** `&` is not drawn; the next glyph is underlined in row 12 across its full advance. Shown by default in Win95 ("&type", "&connection") and Win98 (S, M, T, F, d, x). Win2000 capture shows no underlines. [measured] Row 12 = ascent + 1 for `sserife.fon` 8pt (ascent 11).
- **Win2000 labels in this capture are Tahoma**, so they cannot be matched with `sserife.fon` (at circle+17 / dy 0: 211 and 324 differing pixels; glyph shapes differ, 56 vs 57 px and 97 vs 100 px wide). This is a property of this capture only; Win2000 screens that use `sserife.fon` are not affected. No label origin derived here; the Win2000 sheet labels use `sserife.fon` as an assumption.

## 4. Assumed (carried from the checkbox, no radio capture)
- **Control rectangle:** circle at local x=0 (as the checkbox box), height 16 or 17, circle at local row 2 (both heights), text at local (17, 1) for h=16 and (17, 2) for h=17. Control width = 18 + text width (the focus rectangle's right edge is at 17 + text width). Not measured; only the circle-to-text offsets are.
- **Focus rectangle:** x0 = tx-1 = 16, y0 = ty-1, x1 = tx+textWidth, y1 = ty+14; width textWidth+2, height 16. Dots black where local x+y is odd, local to the control origin. No capture shows a radio focus rectangle (Win98 Traditional has none drawn).
  - h=16: corner local (16,0), even, no dot at the corner. h=17: corner local (16,1), odd, dot at the corner (one extra control row above the rectangle). This replaces the checkbox's "reference starts with a dot" because the radio's rect corner is at x=16, not 17.
  - Like the checkbox's, it is an XOR with a checkerboard brush: black on the face color, other colors on other backgrounds. A replica that draws plain black dots matches only on the face color.
- **Hit area:** the whole control rectangle, no margin; the control may be wider than glyph + label. [wine]
- **Disabled:** interior = face, dot #808080. No disabled radio in any capture, so two label variants are drawn on all three sheets: flat (#808080 drawn once, as the Win95 checkbox capture) and engraved (#FFFFFF copy at +1,+1 under #808080 text, the brief's earlier assumption, which that one checkbox capture contradicted). Which one each version uses for radios is unknown; engraved is required available on all three. **Default to flat**: it is the only disabled label of a classic control that has been measured.
- **RTL:** control mirrored about its width: circle at local x = width-12 (bitmap not flipped), text origin x = width-17-textWidth, focus rect = mirror of the LTR rect, dots by the same local x+y rule. No RTL capture; every RTL pixel here is assumed (including whether the circle is flipped). Labels are Latin because `sserife.fon` has no Hebrew/Arabic glyphs.
- **Pressed (CORRECTED):** interior = face, label normal, and the dot shows the state from **before** the press. Pressing an unselected radio shows the pressed look with no dot, and the previously selected radio in the group keeps its dot. Pressing an already-selected radio shows the pressed look with its dot. Pixels unmeasured; may be dithered.
  - Mouse down: the radio takes focus and the capture, and shows the pressed look. Its check state does not change. [wine: WM_LBUTTONDOWN does SetCapture, SetFocus, BM_SETSTATE TRUE]
  - While held, the pressed look follows the pointer: on inside the control rectangle, off outside, back on when it returns. [wine: WM_MOUSEMOVE while captured]
  - Release inside the control: the radio becomes selected and every other auto radio in its group is cleared, in one step; then BN_CLICKED. [wine: WM_LBUTTONUP → BM_SETCHECK TRUE → BUTTON_CheckAutoRadioButton]
  - Release outside: no state change.
  - After release: normal look (window interior) with the new state. **The focus rectangle stays**: the press gave the radio focus and nothing takes it away.
  - Space bar: the same sequence, with key down as press and key up as release. [wine: WM_KEYDOWN/WM_KEYUP VK_SPACE]

## 5. Not known
- Disabled (flat vs engraved) and pressed pixels, and every RTL pixel, on all three versions (need captures).
- Whether the circle ring uses #DFDFDF-style light on Win98 when disabled.
- Control left edge and height; the focus rectangle on a radio; Win2000 label origin (needs a capture whose labels use `sserife.fon`).
- 3-state does not apply. Group behaviour is out of scope, with one note **[assumed]**: in a dialog, the arrow keys move focus within a radio group and select the radio as it gets focus, with no press/release step. So the keyboard selects immediately and the mouse on release. This is the dialog manager, not the button control.

## 6. Verification (saved PNGs)
Zero mismatches for: saved size, 12x12 key table at every tile, tile crops, focus perimeter parity (both phases), geometry, circle vs capture (every pair-1/pair-2 tile on all three sheets, against that OS's capture), and whole-control-rectangle vs capture for the 4 Win98 replica tiles and, rendered off-sheet, the 2 Win95 capture labels. Win2000: circle only.
Sheet labels are the Win98 capture texts (`&Multiple`, `&Single`, `Te&xtured`, `Soli&d`) on all three sheets so tiles compare across versions. Win2000 mnemonics are drawn although that capture shows none (assumed). Win98 whole-rectangle diff: 16 pixels differ exactly, 0 at tolerance 3 (the capture's #010101 text noise).

## 7. Implementation notes from the replica [replica]
Rendered by `CYTRadioButton` (FOX, Win95 palette, `sserife.fon` 8pt), read back from the X server:
- 8/8 enabled radios match the section 2 table exactly; the dot appears only on the selected one.
- Heights 16 and 17 give text top = circle - 1 and = circle, matching section 3.
- **Redraw when the press ends.** If the replica redraws only when the selection changes, the pressed look stays after release (the selection was already final), and pressing an already-selected radio never shows the pressed look at all. Redraw on every change of the pressed state.
- **Do not change the selection at press.** A toolkit radio that sets itself on press (FOX's `FXRadioButton` does) must draw the pre-press state while held, or two radios show a dot during the press.
- **Known gap:** following the pointer for an already-selected radio needs its own inside/outside tracking. Deriving "inside" from a state flip only works for an unselected radio.
