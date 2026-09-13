/********************************************************************************
*                                                                               *
*                 C Y T B u t t o n   -   W i n 9 5   P u s h   B u t t o n     *
*                                                                               *
*********************************************************************************
* Part of the "Coyote Toolkit" (CYT) widget family -- pixel-accurate replicas  *
* of classic Windows 95/98 (pre-uxtheme) controls, layered on top of FOX.      *
* See PLAN.md on the chicagouireplica branch for the full rendering spec this  *
* class implements.                                                            *
********************************************************************************/
#ifndef CYTBUTTON_H
#define CYTBUTTON_H

#ifndef FXBUTTON_H
#include "FXButton.h"
#endif

namespace FX {


/**
* CYTButton is a pixel-accurate replica of the classic Windows 95/98 push
* button (USER32.DLL-era GDI control painting, identical on Win95/98/NT4,
* superseded by the uxtheme.dll overhaul in XP).
*
* Behaviorally a drop-in replacement for FXButton -- same constructor
* signature, same press/release/default/focus state machine, same message
* IDs -- it only replaces onPaint() with the exact bevel geometry and label
* placement from the Win95 push-button spec. Every other FXButton feature
* (BUTTON_TOOLBAR flat style, icon+text combinations, all four JUSTIFY_*
* corners) is inherited but not specifically replicated by this spec --
* this class targets the plain centered-text dialog push button.
*
* One deliberate behavioral difference from FXButton: BUTTON_DEFAULT is
* always on, regardless of what's passed to the constructor. In a real
* Win95 dialog, the roaming "default button" black outline is automatic
* for every ordinary push button -- not an opt-in setting -- and only
* excludes controls that are a different window class entirely (like
* toolbar buttons). Since CYTButton represents that one specific real
* control, not a general-purpose button that might also be toolbar-
* style, it always participates the same way. BUTTON_INITIAL (which
* marks the one button that starts as default) is unaffected -- still an
* explicit, deliberate per-dialog choice, same as FXButton.
*
* Uses the widget's own theme colors (hiliteColor, shadowColor,
* borderColor, backColor, textColor) rather than hardcoded values, so it
* follows whatever color scheme is active (e.g. ControlPanel's "Redmond
* 95" theme) rather than being pinned to one hardcoded palette.
*
* Disabled-text rendering is configurable: real Win95/98 draws it as flat
* shadowColor text (one pass, no shadow copy), but Windows 2000 changed the
* standard BUTTON control to draw it with a raised, 3D-engraved look (a
* hiliteColor copy offset +1,+1 behind a shadowColor copy on top), matching
* what the Toolbar common control always did. Since both are genuine
* real-Windows behaviors from different eras, which one CYTButton uses is
* read once, at construction, from the registry key SETTINGS/engrave3dstyle
* (set via ControlPanel's General page) -- false (Win95/98 flat gray)
* unless that key says otherwise. See PLAN.md, chicagouireplica branch.
*/
class FXAPI CYTButton : public FXButton {
  FXDECLARE(CYTButton)
protected:
  FXbool engrave3DStyle;
protected:
  CYTButton():engrave3DStyle(false){}
private:
  CYTButton(const CYTButton&);
  CYTButton &operator=(const CYTButton&);
public:
  long onPaint(FXObject*,FXSelector,void*);
public:

  /// Construct button with text and icon; same signature as FXButton
  CYTButton(FXComposite* p,const FXString& text,FXIcon* ic=nullptr,FXObject* tgt=nullptr,FXSelector sel=0,FXuint opts=BUTTON_NORMAL,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_PAD,FXint pr=DEFAULT_PAD,FXint pt=DEFAULT_PAD,FXint pb=DEFAULT_PAD);
  };

}

#endif
