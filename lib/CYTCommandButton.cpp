/********************************************************************************
*                                                                               *
*                 C Y T C o m m a n d B u t t o n   -   W i n 9 5   P u s h   B u t t o n     *
*                                                                               *
*********************************************************************************
* Pixel-accurate replica of the classic (pre-uxtheme) Windows 95/98 push       *
* button, per the "Windows 95/98 Push Button — Rendering Spec" (see PLAN.md    *
* on the chicagouireplica branch). All geometry, offsets, and the -1 vertical  *
* centering correction below are taken directly from that spec, verified      *
* against real Win95/98 rendered output -- not derived independently.         *
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
#include "FXIcon.h"
#include "FXShell.h"
#include "FXButton.h"
#include "CYTCommandButton.h"

using namespace FX;

namespace FX {

// Map -- only onPaint is overridden; every other FXButton behavior (press/
// release, default/focus state machine, message IDs) is inherited unchanged.
FXDEFMAP(CYTCommandButton) CYTCommandButtonMap[]={
  FXMAPFUNC(SEL_PAINT,0,CYTCommandButton::onPaint),
  };


// Object implementation
FXIMPLEMENT(CYTCommandButton,FXButton,CYTCommandButtonMap,ARRAYNUMBER(CYTCommandButtonMap))


// Construct button with text and icon; same signature as FXButton.
//
// BUTTON_DEFAULT is forced on unconditionally, regardless of what the
// caller passes in opts. In real Win95, the roaming "default button"
// outline (see FXWindow::setDefault()/FXButton::setFocus()/killFocus(),
// all inherited unchanged) is not an opt-in per-control setting -- it's
// automatic for any BS_PUSHBUTTON-class control in a dialog, tracked by
// the dialog manager regardless of what the app author does. FXButton
// needs BUTTON_DEFAULT as an explicit flag because one FXButton class
// also serves toolbar-style buttons (BUTTON_TOOLBAR) that must never
// participate. CYTCommandButton has no such dual role -- it represents exactly
// one real control, the classic push button -- so it always
// participates, the same way a real one always would. A future toolbar-
// style Coyote widget simply won't inherit from CYTCommandButton, the same way
// Win95's toolbar buttons are a different window class entirely, not a
// flag on the same one (see PLAN.md, chicagouireplica branch).
// 3D-engraved style (flat Win95/98 gray vs. engraved Win2000 disabled
// text) is read once here from the registry, same "takes effect next
// app launch" convention as ControlPanel's other SETTINGS-section
// entries (e.g. UI Scaling). See the doc comment in CYTCommandButton.h.
CYTCommandButton::CYTCommandButton(FXComposite* p,const FXString& text,FXIcon* ic,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb):
  FXButton(p,text,ic,tgt,sel,opts|BUTTON_DEFAULT,x,y,w,h,pl,pr,pt,pb),
  engrave3DStyle(getApp()->reg().readBoolEntry("SETTINGS","engrave3dstyle",false)),
  themeStyle(FXCLAMP(CYT_STYLE_95,getApp()->reg().readIntEntry("SETTINGS","themestyle",CYT_STYLE_95),CYT_STYLE_2000)){
  }


/*******************************************************************************/

// Draw the raised ("Normal") bevel into an arbitrary sub-rectangle of the
// canvas -- reused as-is for the BUTTON_DEFAULT outline (spec section 5,
// "Default": same bevel, just painted 1px inset on all sides). Spec section
// 5, "Normal", steps 1-7.
static void cytDrawRaisedBevel(FXDCWindow& dc,FXColor face,FXColor hilite,FXColor shadow,FXColor shadowDark,FXint x,FXint y,FXint w,FXint h){
  if(w<2 || h<2) return;
  dc.setForeground(face);
  dc.fillRectangle(x,y,w,h);
  dc.setForeground(hilite);
  dc.fillRectangle(x,y,w-1,1);          // top edge,  x=0..W-2
  dc.fillRectangle(x,y,1,h-1);          // left edge, y=0..H-2
  dc.setForeground(shadowDark);
  dc.fillRectangle(x,y+h-1,w,1);        // bottom edge (outer), full width
  dc.fillRectangle(x+w-1,y,1,h);        // right edge (outer), full height
  dc.setForeground(shadow);
  dc.fillRectangle(x+1,y+h-2,w-2,1);    // inner bottom edge, x=1..W-2
  dc.fillRectangle(x+w-2,y+1,1,h-2);    // inner right edge,  y=1..H-2
  }


// Per-channel average, rounded down -- how Windows derives COLOR_3DLIGHT
// (face+highlight) and COLOR_3DDKSHADOW (windowframe+shadow) (spec section 6)
static FXColor cytAverage(FXColor a,FXColor b){
  return FXRGB((FXREDVAL(a)+FXREDVAL(b))/2,(FXGREENVAL(a)+FXGREENVAL(b))/2,(FXBLUEVAL(a)+FXBLUEVAL(b))/2);
  }


// Windows 98's extra inner highlight ring (spec section 6): one more line
// just inside the bevel's top/left edge, stopping short of the shadow
// columns. inset is 1 for a plain bevel, 2 for a Default button's (whose
// bevel is itself inset 1px inside the black ring).
static void cytDrawInnerHighlight(FXDCWindow& dc,FXColor light,FXint inset,FXint w,FXint h){
  FXint len_x=w-2*inset-1,len_y=h-2*inset-1;
  if(len_x<=0 || len_y<=0) return;
  dc.setForeground(light);
  dc.fillRectangle(inset,inset,len_x,1);   // top,  x=inset..W-inset-2
  dc.fillRectangle(inset,inset,1,len_y);   // left, y=inset..H-inset-2
  }


// Draw the flat, symmetric "Pressed" double-dark frame (spec section 5,
// "Pressed") -- no highlight anywhere, unlike the raised bevel above.
static void cytDrawPressedBevel(FXDCWindow& dc,FXColor face,FXColor shadow,FXColor shadowDark,FXint x,FXint y,FXint w,FXint h){
  if(w<2 || h<2) return;
  dc.setForeground(face);
  dc.fillRectangle(x,y,w,h);
  dc.setForeground(shadowDark);
  dc.fillRectangle(x,y,w,1);            // outer top
  dc.fillRectangle(x,y+h-1,w,1);        // outer bottom
  dc.fillRectangle(x,y,1,h);            // outer left
  dc.fillRectangle(x+w-1,y,1,h);        // outer right
  dc.setForeground(shadow);
  dc.fillRectangle(x+1,y+1,w-2,1);      // inner top,    x=1..W-2
  dc.fillRectangle(x+1,y+h-2,w-2,1);    // inner bottom, x=1..W-2
  dc.fillRectangle(x+1,y+1,1,h-2);      // inner left,   y=1..H-2
  dc.fillRectangle(x+w-2,y+1,1,h-2);    // inner right,  y=1..H-2
  }


// Draw the dotted focus rectangle (spec section 5, "Focus rectangle"):
// inset 4px on every side, perimeter traced once (corners not double-
// counted), alternating transparent/shadowDark starting transparent at the
// very first point. Deliberately not reusing FXDCWindow::drawFocusRectangle()
// -- that one uses a GC-level stipple pattern anchored to screen/window
// coordinates, not to the rectangle's own perimeter, so it can't guarantee
// "starts transparent, closes with no seam" the way this spec requires.
static void cytDrawFocusRect(FXDCWindow& dc,FXColor shadowDark,FXint w,FXint h){
  FXint x0=4,y0=4,x1=w-5,y1=h-5,x,y,idx=0;
  if(x1<=x0 || y1<=y0) return;
  dc.setForeground(shadowDark);
  for(x=x0; x<=x1; ++x){ if(idx&1) dc.fillRectangle(x,y0,1,1); ++idx; }        // top,    left->right
  for(y=y0+1; y<=y1; ++y){ if(idx&1) dc.fillRectangle(x1,y,1,1); ++idx; }      // right,  top->bottom
  for(x=x1-1; x>=x0; --x){ if(idx&1) dc.fillRectangle(x,y1,1,1); ++idx; }      // bottom, right->left
  for(y=y1-1; y>=y0+1; --y){ if(idx&1) dc.fillRectangle(x0,y,1,1); ++idx; }    // left,   bottom->top
  }


/*******************************************************************************/

// Handle repaint
long CYTCommandButton::onPaint(FXObject*,FXSelector,void* ptr){
  FXEvent *ev=(FXEvent*)ptr;
  FXDCWindow dc(this,ev);

  // Spec section 4: label placement, computed once against the full W x H
  // button rect regardless of state (Default's 1px inset is symmetric, so
  // it lands on the same pixel either way -- see spec section 5, "Default").
  FXint textWidth=label.empty() ? 0 : font->getTextWidth(label);
  FXint lineHeight=font->getFontHeight();
  FXint ascent=font->getFontAscent();
  FXint x=(width-textWidth)/2;                    // horizontal: exact, no correction
  FXint y=(height/2)-(lineHeight/2)-1;             // vertical: formula, then the required -1
  FXint xp=x+1,yp=y+1;                             // pressed offset: flat translation of both axes

  FXbool pressed=isEnabled() && (state!=STATE_UP);
  FXbool deflt=isDefault() && isEnabled() && !pressed;
  FXbool focused=hasFocus();

  // Spec section 6: Win2000 darkens the bevel's own outer bottom/right edge
  // (never the pressed frame, nor Default's added black ring); Win98 adds
  // an inner top/left highlight ring. Both colors are computed from theme.
  FXColor outerEdge=(themeStyle==CYT_STYLE_2000) ? cytAverage(borderColor,shadowColor) : borderColor;
  FXColor light3D=cytAverage(backColor,hiliteColor);

  // Spec section 5: border/bevel, per state
  if(pressed){
    cytDrawPressedBevel(dc,backColor,shadowColor,borderColor,0,0,width,height);
    }
  else if(deflt){
    dc.setForeground(borderColor);
    dc.fillRectangle(0,0,width,height);
    cytDrawRaisedBevel(dc,backColor,hiliteColor,shadowColor,outerEdge,1,1,width-2,height-2);
    if(themeStyle==CYT_STYLE_98) cytDrawInnerHighlight(dc,light3D,2,width,height);
    }
  else{
    cytDrawRaisedBevel(dc,backColor,hiliteColor,shadowColor,outerEdge,0,0,width,height);
    if(themeStyle==CYT_STYLE_98) cytDrawInnerHighlight(dc,light3D,1,width,height);
    }

  // Spec section 5: label text, per state
  if(!label.empty()){
    dc.setFont(font);
    if(!isEnabled()){
      if(engrave3DStyle){
        // Engraved (Windows 2000 style): hiliteColor copy offset +1,+1
        // behind a shadowColor copy on top -- Win2000 changed the standard
        // BUTTON control to match what the Toolbar common control always
        // did (see engrave3DStyle's doc comment in CYTCommandButton.h).
        dc.setForeground(hiliteColor);
        dc.drawText(x+1,y+1+ascent,label);
        dc.setForeground(shadowColor);
        dc.drawText(x,y+ascent,label);
        }
      else{
        // Flat gray, single pass (Windows 95/98 style) -- the real Win95
        // BUTTON window class's disabled text (SetTextColor(COLOR_GRAYTEXT)
        // then one DrawText). See PLAN.md, chicagouireplica branch.
        dc.setForeground(shadowColor);
        dc.drawText(x,y+ascent,label);
        }
      }
    else if(pressed){
      dc.setForeground(textColor);
      dc.drawText(xp,yp+ascent,label);
      }
    else{
      dc.setForeground(textColor);
      dc.drawText(x,y+ascent,label);
      }
    }

  // Spec section 5: focus rectangle -- overlay, drawn last, only while
  // actually focused (independent of Default -- a plain button can be
  // focused without being the dialog's default, and vice versa)
  if(isEnabled() && focused){
    cytDrawFocusRect(dc,borderColor,width,height);
    }

  return 1;
  }

}
