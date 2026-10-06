/********************************************************************************
*                                                                               *
*               C Y T R a d i o B u t t o n   -   W i n 9 5   R a d i o        *
*                                                                               *
*********************************************************************************
* Part of the "Coyote Toolkit" (CYT) widget family -- pixel-accurate replicas  *
* of classic Windows 95/98 (pre-uxtheme) controls, layered on top of FOX.      *
* Implements the "Radio button spec: Win95 / Win98 / Win2000 classic          *
* controls" (measured from real captures; see the class doc comment for       *
* which parts of that spec are measured and which are assumed).               *
********************************************************************************/
#ifndef CYTRADIOBUTTON_H
#define CYTRADIOBUTTON_H

#ifndef FXRADIOBUTTON_H
#include "FXRadioButton.h"
#endif

#ifndef CYTCOMMANDBUTTON_H
#include "CYTCommandButton.h"           // CYT_STYLE_95/98/2000
#endif

namespace FX {


/**
* CYTRadioButton is a pixel-accurate replica of the classic Windows
* 95/98/2000 radio button (BS_RADIOBUTTON/BS_AUTORADIOBUTTON -- USER32.DLL-
* era GDI control painting, superseded by the uxtheme.dll overhaul in XP).
*
* Behaviorally a drop-in replacement for FXRadioButton -- same constructor
* signature, same check/press/keyboard/hotkey state machine, same message
* IDs -- it replaces onPaint() and the default size, and repaints whenever
* a press starts or ends (FXRadioButton only repaints on a `check` change,
* so the pressed look would otherwise stick after release, or never show
* when pressing an already-selected radio). While pressed it draws the
* state from before the press, as Windows does: the selection only moves
* on release, so a held, unselected radio shows the pressed look with no
* dot, and the old selection keeps its dot until then.
*
* Measured from captures (spec sections 2-3): the 12x12 circle bitmap
* (identical on all three versions; only its colors differ), the 4x4 dot,
* and the label origin 17px right of the circle's left edge. Carried over
* as assumptions (spec section 4, no radio capture): the control rectangle
* (circle at local x=0; circle and text each centered vertically by
* integer division, which reproduces the measured 0/-1 text offset for
* odd/even control heights), the focus rectangle, the disabled colors,
* and the pressed look. RTL layout (spec section 4) is not implemented.
*
* Colors come from the widget's own theme colors, never hardcoded:
*   outer top-left  = shadowColor
*   inner top-left  = borderColor (95/98), or (borderColor+shadowColor)/2
*                     for CYT_STYLE_2000 (#404040, 3DDKSHADOW)
*   inner bot-right = backColor (95/2000), or (backColor+hiliteColor)/2
*                     for CYT_STYLE_98 (#DFDFDF, 3DLIGHT)
*   outer bot-right = hiliteColor
*   interior        = diskColor (window color); backColor when disabled
*                     or pressed
*   dot             = radioColor; shadowColor when disabled
*
* The control rectangle is the widget minus its padding and border, so
* pass zero padding for the exact Windows geometry (the default pads are
* kept only so the constructor stays a drop-in for FXRadioButton's).
*
* Border style (SETTINGS/themestyle) and disabled-text style (SETTINGS/
* engrave3dstyle, flat gray vs. engraved) are read once at construction,
* the same keys and convention as CYTCommandButton.
*/
class FXAPI CYTRadioButton : public FXRadioButton {
  FXDECLARE(CYTRadioButton)
protected:
  FXbool engrave3DStyle;
  FXint  themeStyle;
protected:
  CYTRadioButton():engrave3DStyle(false),themeStyle(CYT_STYLE_95){}
private:
  CYTRadioButton(const CYTRadioButton&);
  CYTRadioButton &operator=(const CYTRadioButton&);
public:
  long onPaint(FXObject*,FXSelector,void*);
  long onPressState(FXObject*,FXSelector,void*);
public:

  /// Construct radio button; same signature as FXRadioButton
  CYTRadioButton(FXComposite* p,const FXString& text,FXObject* tgt=nullptr,FXSelector sel=0,FXuint opts=RADIOBUTTON_NORMAL,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_PAD,FXint pr=DEFAULT_PAD,FXint pt=DEFAULT_PAD,FXint pb=DEFAULT_PAD);

  /// Get default width: 18 + text width (spec section 4)
  virtual FXint getDefaultWidth();

  /// Get default height: 16 for a 13px text line (spec section 4)
  virtual FXint getDefaultHeight();
  };

}

#endif
