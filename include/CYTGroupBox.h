/********************************************************************************
*                                                                               *
*               C Y T G r o u p B o x   -   W i n 9 5   G r o u p   B o x      *
*                                                                               *
*********************************************************************************
* Part of the "Coyote Toolkit" (CYT) widget family -- pixel-accurate replicas  *
* of classic Windows 95/98 (pre-uxtheme) controls, layered on top of FOX.      *
* See PLAN.md on the chicagouireplica branch for the full rendering spec this  *
* class implements ("Windows 95/98/2000 group box spec -- basic").            *
********************************************************************************/
#ifndef CYTGROUPBOX_H
#define CYTGROUPBOX_H

#ifndef FXGROUPBOX_H
#include "FXGroupBox.h"
#endif

namespace FX {


/**
* CYTGroupBox is a pixel-accurate replica of the classic Windows 95/98/2000
* group box (BS_GROUPBOX -- USER32.DLL-era GDI control painting, superseded
* by the uxtheme.dll overhaul in XP).
*
* Behaviorally a drop-in replacement for FXGroupBox -- same constructor
* signature, same GROUPBOX_TITLE_LEFT/CENTER/RIGHT alignment options, same
* child-layout behavior (inherited unchanged) -- it only replaces onPaint()
* with the exact etched-frame geometry and label placement from the group
* box spec. Unlike a button, the frame is *etched* (two concentric
* highlight/shadow rings, never SHADOW_DARK) rather than raised, and the
* label is top-aligned (y=0), never vertically centered -- the frame's top
* ring is pushed down by font.lineHeight/2 to make room for it, with a gap
* cut into both top-ring rows under the label's run (or drawn unbroken when
* there's no label).
*
* Uses the widget's own theme colors (hiliteColor, shadowColor, backColor,
* textColor) rather than hardcoded values, so it follows whatever color
* scheme is active rather than being pinned to one hardcoded palette. This
* basic spec covers the 95/2000 look identically (the frame never uses
* SHADOW_DARK or COLOR_3DDKSHADOW to begin with, so there's nothing for a
* Win2000 "themeStyle" to change) -- a Win98 variant would add an extra
* COLOR_3DLIGHT inner ring, but that's explicitly out of scope for this
* basic spec and not implemented here.
*
* Disabled-text style (flat Win95/98 gray vs. engraved Win2000) mirrors
* CYTCommandButton's: read once, at construction, from the registry key
* SETTINGS/engrave3dstyle -- false (flat gray) unless that key says
* otherwise.
*/
class FXAPI CYTGroupBox : public FXGroupBox {
  FXDECLARE(CYTGroupBox)
protected:
  FXbool engrave3DStyle;
protected:
  CYTGroupBox():engrave3DStyle(false){}
private:
  CYTGroupBox(const CYTGroupBox&);
  CYTGroupBox &operator=(const CYTGroupBox&);
public:
  long onPaint(FXObject*,FXSelector,void*);
public:

  /// Construct group box; same signature as FXGroupBox
  CYTGroupBox(FXComposite* p,const FXString& text,FXuint opts=GROUPBOX_NORMAL,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_SPACING,FXint pr=DEFAULT_SPACING,FXint pt=DEFAULT_SPACING,FXint pb=DEFAULT_SPACING,FXint hs=DEFAULT_SPACING,FXint vs=DEFAULT_SPACING);
  };

}

#endif
