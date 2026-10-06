/********************************************************************************
*                                                                               *
*                  C Y T C h e c k B o x   -   W i n 9 5   C h e c k b o x      *
*                                                                               *
*********************************************************************************
* Part of the "Coyote Toolkit" (CYT) widget family -- pixel-accurate replicas  *
* of classic Windows 95/98 (pre-uxtheme) controls, layered on top of FOX.      *
* Implements the "Checkbox control" brief (measured from real captures; see   *
* the class doc comment for which parts are measured and which are assumed).  *
********************************************************************************/
#ifndef CYTCHECKBOX_H
#define CYTCHECKBOX_H

#ifndef FXCHECKBUTTON_H
#include "FXCheckButton.h"
#endif

#ifndef CYTCOMMANDBUTTON_H
#include "CYTCommandButton.h"           // CYT_STYLE_95/98/2000
#endif

namespace FX {


/**
* CYTCheckBox is a pixel-accurate replica of the classic Windows 95/98/2000
* check box (BS_CHECKBOX/BS_AUTOCHECKBOX -- USER32.DLL-era GDI control
* painting, superseded by the uxtheme.dll overhaul in XP).
*
* Behaviorally a drop-in replacement for FXCheckButton -- same constructor
* signature, same check/press/keyboard/hotkey state machine, same message
* IDs -- it replaces onPaint() and the default size only.
*
* Measured from captures: the 13x13 box (identical on all three versions;
* only its colors differ), the tick, the flat (not engraved) disabled
* label, and the label origin 18px right of the box's left edge. Carried
* over as assumptions: the control rectangle (box at local x=0; box and
* text each centered vertically by integer division, which reproduces
* both measured text offsets -- box-1 for a 16px control, box+0 for 17px),
* the pressed look, and the third (MAYBE) state. RTL layout is not
* implemented.
*
* While pressed, the box shows the pressed look and the tick of the state
* *before* the press, as Windows does (it only toggles on release);
* FXCheckButton itself toggles `check` immediately on press, so the paint
* reads `oldcheck` instead while FLAG_PRESSED is set. The pressed look
* tracks the pointer, because FXCheckButton's onEnter/onLeave already
* toggle `check` back and forth relative to `oldcheck`. Every press/
* release message also repaints when it changes FLAG_PRESSED, since
* FXCheckButton only repaints on a `check` change -- without that, a
* release (where `check` is already final) would leave the pressed look
* on screen.
*
* Colors come from the widget's own theme colors, never hardcoded:
*   outer top-left  = shadowColor
*   inner top-left  = borderColor (95/98), or (borderColor+shadowColor)/2
*                     for CYT_STYLE_2000 (#404040, 3DDKSHADOW)
*   inner bot-right = backColor (95/2000), or (backColor+hiliteColor)/2
*                     for CYT_STYLE_98 (#DFDFDF, 3DLIGHT)
*   outer bot-right = hiliteColor
*   interior        = boxColor (window color); backColor when disabled,
*                     pressed, or in the MAYBE state
*   tick            = checkColor; shadowColor when disabled or MAYBE
*
* The control rectangle is the widget minus its padding and border, so
* pass zero padding for the exact Windows geometry (the default pads are
* kept only so the constructor stays a drop-in for FXCheckButton's).
*
* Border style (SETTINGS/themestyle) and disabled-text style (SETTINGS/
* engrave3dstyle; flat gray is the measured Win95 look) are read once at
* construction, the same keys and convention as CYTCommandButton.
*/
class FXAPI CYTCheckBox : public FXCheckButton {
  FXDECLARE(CYTCheckBox)
protected:
  FXbool engrave3DStyle;
  FXint  themeStyle;
protected:
  CYTCheckBox():engrave3DStyle(false),themeStyle(CYT_STYLE_95){}
private:
  CYTCheckBox(const CYTCheckBox&);
  CYTCheckBox &operator=(const CYTCheckBox&);
public:
  long onPaint(FXObject*,FXSelector,void*);
  long onPressState(FXObject*,FXSelector,void*);
public:

  /// Construct check box; same signature as FXCheckButton
  CYTCheckBox(FXComposite* p,const FXString& text,FXObject* tgt=nullptr,FXSelector sel=0,FXuint opts=CHECKBUTTON_NORMAL,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_PAD,FXint pr=DEFAULT_PAD,FXint pt=DEFAULT_PAD,FXint pb=DEFAULT_PAD);

  /// Get default width: 19 + text width, so the focus rectangle fits
  virtual FXint getDefaultWidth();

  /// Get default height: 16 for a 13px text line
  virtual FXint getDefaultHeight();
  };

}

#endif
