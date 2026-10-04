/********************************************************************************
*                                                                               *
*               C Y T G r o u p B o x   -   W i n 9 5   G r o u p   B o x      *
*                                                                               *
*********************************************************************************
* Pixel-accurate replica of the classic (pre-uxtheme) Windows 95/98/2000       *
* group box, per the "Windows 95/98/2000 group box spec -- basic" (see        *
* PLAN.md on the chicagouireplica branch). All geometry and offsets below are *
* taken directly from that spec, verified against real Win95/98/2000          *
* rendered output -- not derived independently.                               *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "fxmath.h"
#include "fxkeys.h"
#include "FXMutex.h"
#include "FXSize.h"
#include "FXPoint.h"
#include "FXRectangle.h"
#include "FXElement.h"
#include "FXMetaClass.h"
#include "FXHash.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXStringDictionary.h"
#include "FXSettings.h"
#include "FXRegistry.h"
#include "FXEvent.h"
#include "FXWindow.h"
#include "FXDCWindow.h"
#include "FXApp.h"
#include "FXFont.h"
#include "FXPacker.h"
#include "FXGroupBox.h"
#include "CYTGroupBox.h"

using namespace FX;

namespace FX {

#define GROUPBOX_TITLE_MASK (GROUPBOX_TITLE_LEFT|GROUPBOX_TITLE_CENTER|GROUPBOX_TITLE_RIGHT)

// Map -- only onPaint is overridden; everything else (layout, enable/
// disable, setText/setFont/setTextColor, the ID_SETVALUE family) is
// inherited unchanged from FXGroupBox.
FXDEFMAP(CYTGroupBox) CYTGroupBoxMap[]={
  FXMAPFUNC(SEL_PAINT,0,CYTGroupBox::onPaint),
  };


// Object implementation
FXIMPLEMENT(CYTGroupBox,FXGroupBox,CYTGroupBoxMap,ARRAYNUMBER(CYTGroupBoxMap))


// Construct group box; same signature as FXGroupBox. Disabled-text style
// (flat Win95/98 gray vs. engraved Win2000) is read once here from the
// registry, same key and "takes effect next app launch" convention as
// CYTCommandButton -- see the doc comment in CYTGroupBox.h.
CYTGroupBox::CYTGroupBox(FXComposite* p,const FXString& text,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb,FXint hs,FXint vs):
  FXGroupBox(p,text,opts,x,y,w,h,pl,pr,pt,pb,hs,vs),
  engrave3DStyle(getApp()->reg().readBoolEntry("SETTINGS","engrave3dstyle",false)){
  }


/*******************************************************************************/

// Draw the etched frame (spec section 5): two concentric 1px rings, always
// 1px apart -- outer SHADOW on top/left, HIGHLIGHT on bottom/right; inner
// the reverse. top is the frame's top edge (FRAME_TOP, spec section 4);
// the left/right edges run from top down to H-1, never from 0. When
// gapLo<gapHi, both top-ring rows (outer and inner) skip that x-range
// (spec section 6) -- pass gapLo==gapHi (or any empty range) for no label.
static void cytDrawEtchedFrame(FXDCWindow& dc,FXColor hilite,FXColor shadow,FXint top,FXint w,FXint h,FXint gapLo,FXint gapHi){
  FXint x,y;
  if(w<2 || h<2 || top>=h-1) return;

  // Outer top row and inner top row, both label-gap-aware
  dc.setForeground(shadow);
  for(x=0; x<w; ++x){ if(x<gapLo || gapHi<=x) dc.fillRectangle(x,top,1,1); }
  dc.setForeground(hilite);
  for(x=1; x<w-1; ++x){ if(x<gapLo || gapHi<=x) dc.fillRectangle(x,top+1,1,1); }

  // Outer left (shortened to start at top) and inner left
  dc.setForeground(shadow);
  dc.fillRectangle(0,top,1,h-top);
  dc.setForeground(hilite);
  dc.fillRectangle(1,top+1,1,h-top-2);

  // Outer bottom (fixed, unaffected by the label shift) and inner bottom
  dc.setForeground(hilite);
  dc.fillRectangle(0,h-1,w,1);
  dc.setForeground(shadow);
  dc.fillRectangle(1,h-2,w-2,1);

  // Outer right (shortened) and inner right
  dc.setForeground(hilite);
  dc.fillRectangle(w-1,top,1,h-top);
  dc.setForeground(shadow);
  dc.fillRectangle(w-2,top+1,1,h-top-2);
  }


/*******************************************************************************/

// Handle repaint
long CYTGroupBox::onPaint(FXObject*,FXSelector,void* ptr){
  FXEvent *ev=(FXEvent*)ptr;
  FXDCWindow dc(this,ev);

  // Spec section 2: no default/pressed states, so the frame never uses
  // SHADOW_DARK -- fill with the widget's own theme back color (FACE)
  dc.setForeground(backColor);
  dc.fillRectangle(0,0,width,height);

  // Spec section 4: the frame's top edge is pushed down by lineHeight/2,
  // unconditionally -- even with no label at all (truncating division)
  FXint top=font->getFontHeight()/2;

  // Spec section 6: label placement, per GROUPBOX_TITLE_LEFT/CENTER/RIGHT
  // (already an FXGroupBox option -- reused as-is, not reinvented here)
  FXint x=0,gapLo=0,gapHi=0;
  FXbool hasLabel=!label.empty();
  if(hasLabel){
    FXint textWidth=font->getTextWidth(label);
    switch(options&GROUPBOX_TITLE_MASK){
      case GROUPBOX_TITLE_CENTER: x=(width/2)-(textWidth/2); break;
      case GROUPBOX_TITLE_RIGHT:  x=width-9-textWidth; break;
      default:                    x=9; break;        // GROUPBOX_TITLE_LEFT
      }
    gapLo=x-2;
    gapHi=x+textWidth+2;
    }

  cytDrawEtchedFrame(dc,hiliteColor,shadowColor,top,width,height,gapLo,gapHi);

  // Spec section 6/7: label text -- always y=0 (top-aligned, never
  // vertically centered like a button's label), per state
  if(hasLabel){
    dc.setFont(font);
    if(!isEnabled()){
      if(engrave3DStyle){
        // Engraved (Windows 2000 style) -- see CYTCommandButton's Disabled
        // (engraved) state for the same two-pass treatment
        dc.setForeground(hiliteColor);
        dc.drawText(x+1,1+font->getFontAscent(),label);
        dc.setForeground(shadowColor);
        dc.drawText(x,font->getFontAscent(),label);
        }
      else{
        // Flat gray, single pass (Windows 95/98 style)
        dc.setForeground(shadowColor);
        dc.drawText(x,font->getFontAscent(),label);
        }
      }
    else{
      dc.setForeground(textColor);
      dc.drawText(x,font->getFontAscent(),label);
      }
    }

  return 1;
  }

}
