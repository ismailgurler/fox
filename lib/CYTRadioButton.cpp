/********************************************************************************
*                                                                               *
*               C Y T R a d i o B u t t o n   -   W i n 9 5   R a d i o        *
*                                                                               *
*********************************************************************************
* Pixel-accurate replica of the classic (pre-uxtheme) Windows 95/98/2000       *
* radio button, per the "Radio button spec: Win95 / Win98 / Win2000 classic   *
* controls". The circle bitmap, dot, and label offset below are taken         *
* directly from that spec's measured tables; the control rectangle, focus     *
* rectangle, disabled and pressed looks are its stated assumptions.           *
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
#include "FXLabel.h"
#include "FXRadioButton.h"
#include "CYTRadioButton.h"

using namespace FX;

namespace FX {

// Map -- onPaint is overridden; the press/release messages are routed
// through onPressState, which runs FXRadioButton's own handler unchanged
// and only adds a repaint. Every other FXRadioButton behavior is inherited.
FXDEFMAP(CYTRadioButton) CYTRadioButtonMap[]={
  FXMAPFUNC(SEL_PAINT,0,CYTRadioButton::onPaint),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,CYTRadioButton::onPressState),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,CYTRadioButton::onPressState),
  FXMAPFUNC(SEL_UNGRABBED,0,CYTRadioButton::onPressState),
  FXMAPFUNC(SEL_KEYPRESS,0,CYTRadioButton::onPressState),
  FXMAPFUNC(SEL_KEYRELEASE,0,CYTRadioButton::onPressState),
  FXMAPFUNC(SEL_KEYPRESS,FXWindow::ID_HOTKEY,CYTRadioButton::onPressState),
  FXMAPFUNC(SEL_KEYRELEASE,FXWindow::ID_HOTKEY,CYTRadioButton::onPressState),
  };


// Object implementation
FXIMPLEMENT(CYTRadioButton,FXRadioButton,CYTRadioButtonMap,ARRAYNUMBER(CYTRadioButtonMap))


// Spec section 2: circle size, and the label's offset from the circle's
// left edge (spec section 3, measured -- not the checkbox's 18)
#define CIRCLE_SIZE   12
#define LABEL_OFFSET  17


// Spec section 2: the 12x12 circle, local (col,row), identical on Win95,
// Win98 and Win2000 (only the colors differ). G outer top-left, K inner
// top-left, L inner bottom-right, W outer bottom-right, i interior,
// '.' outside the circle (left untouched: already the face color).
// Not a mirror image: row 2 has KK at cols 8-9 and W at col 10.
static const FXchar circleBitmap[CIRCLE_SIZE][CIRCLE_SIZE+1]={
  "....GGGG....",
  "..GGKKKKGG..",
  ".GKKiiiiKKW.",
  ".GKiiiiiiLW.",
  "GKiiiiiiiiLW",
  "GKiiiiiiiiLW",
  "GKiiiiiiiiLW",
  "GKiiiiiiiiLW",
  ".GKiiiiiiLW.",
  ".GLLiiiiLLW.",
  "..WWLLLLWW..",
  "....WWWW....",
  };


// Per-channel average, rounded down -- how Windows derives COLOR_3DLIGHT
// (face+highlight) and COLOR_3DDKSHADOW (windowframe+shadow); same formula
// as CYTCommandButton's
static FXColor cytAverage(FXColor a,FXColor b){
  return FXRGB((FXREDVAL(a)+FXREDVAL(b))/2,(FXGREENVAL(a)+FXGREENVAL(b))/2,(FXBLUEVAL(a)+FXBLUEVAL(b))/2);
  }


// Construct radio button; same signature as FXRadioButton. Border and
// disabled-text styles are read once here from the registry, same keys
// and "takes effect next app launch" convention as CYTCommandButton.
CYTRadioButton::CYTRadioButton(FXComposite* p,const FXString& text,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb):
  FXRadioButton(p,text,tgt,sel,opts,x,y,w,h,pl,pr,pt,pb),
  engrave3DStyle(getApp()->reg().readBoolEntry("SETTINGS","engrave3dstyle",false)),
  themeStyle(FXCLAMP(CYT_STYLE_95,getApp()->reg().readIntEntry("SETTINGS","themestyle",CYT_STYLE_95),CYT_STYLE_2000)){
  }


/*******************************************************************************/

// Get default width -- spec section 4: control width = 18 + text width
// (text starts at 17, and the focus rectangle's right edge is one pixel
// past the text, at 17+textWidth)
FXint CYTRadioButton::getDefaultWidth(){
  FXint w=label.empty() ? CIRCLE_SIZE : LABEL_OFFSET+1+labelWidth(label);
  return padleft+padright+w+(border<<1);
  }


// Get default height -- spec section 4: 16 for sserife.fon's 13px line
// (the focus rectangle needs lineHeight+3 rows), never less than the circle
FXint CYTRadioButton::getDefaultHeight(){
  FXint h=FXMAX(font->getFontHeight(),CIRCLE_SIZE)+3;
  return padtop+padbottom+h+(border<<1);
  }


/*******************************************************************************/

// Press/release (mouse, space, hotkey, lost grab): let FXRadioButton
// handle it exactly as before, then repaint if FLAG_PRESSED changed --
// FXRadioButton only repaints when `check` changes, so the pressed look
// would stick after release, or never show on an already-selected radio
long CYTRadioButton::onPressState(FXObject* sender,FXSelector sel,void* ptr){
  FXuint was=flags&FLAG_PRESSED;
  long res=FXRadioButton::handle(sender,sel,ptr);
  if((flags&FLAG_PRESSED)!=was) update();
  return res;
  }


/*******************************************************************************/

// Draw the circle bitmap (spec section 2) at (x,y), one color per key
// letter, merging horizontal runs of the same key into one fill
static void cytDrawCircle(FXDCWindow& dc,FXint x,FXint y,FXColor outerTL,FXColor innerTL,FXColor innerBR,FXColor outerBR,FXColor interior){
  FXint row,col,end;
  FXchar key;
  for(row=0; row<CIRCLE_SIZE; ++row){
    for(col=0; col<CIRCLE_SIZE; col=end){
      key=circleBitmap[row][col];
      for(end=col+1; end<CIRCLE_SIZE && circleBitmap[row][end]==key; ++end){}
      switch(key){
        case 'G': dc.setForeground(outerTL); break;
        case 'K': dc.setForeground(innerTL); break;
        case 'L': dc.setForeground(innerBR); break;
        case 'W': dc.setForeground(outerBR); break;
        case 'i': dc.setForeground(interior); break;
        default: continue;
        }
      dc.fillRectangle(x+col,y+row,end-col,1);
      }
    }
  }


// Draw the checked dot (spec section 2): 4x4 with the corners off --
// row 4 cols 5-6, rows 5-6 cols 4-7, row 7 cols 5-6, circle-local
static void cytDrawDot(FXDCWindow& dc,FXint x,FXint y,FXColor color){
  dc.setForeground(color);
  dc.fillRectangle(x+5,y+4,2,1);
  dc.fillRectangle(x+4,y+5,4,2);
  dc.fillRectangle(x+5,y+7,2,1);
  }


// Draw the dotted focus rectangle (spec section 4): perimeter of
// (x0,y0)-(x1,y1) inclusive, a dot wherever x+y is odd in coordinates
// local to the control origin (ox,oy) -- so whether a corner gets a dot
// depends on the control height's parity, as the spec describes
static void cytDrawFocusRect(FXDCWindow& dc,FXColor color,FXint ox,FXint oy,FXint x0,FXint y0,FXint x1,FXint y1){
  FXint x,y;
  if(x1<=x0 || y1<=y0) return;
  dc.setForeground(color);
  for(x=x0; x<=x1; ++x){
    if(((x-ox)+(y0-oy))&1) dc.fillRectangle(x,y0,1,1);
    if(((x-ox)+(y1-oy))&1) dc.fillRectangle(x,y1,1,1);
    }
  for(y=y0+1; y<y1; ++y){
    if(((x0-ox)+(y-oy))&1) dc.fillRectangle(x0,y,1,1);
    if(((x1-ox)+(y-oy))&1) dc.fillRectangle(x1,y,1,1);
    }
  }


/*******************************************************************************/

// Handle repaint
long CYTRadioButton::onPaint(FXObject*,FXSelector,void* ptr){
  FXEvent *ev=(FXEvent*)ptr;
  FXDCWindow dc(this,ev);

  dc.setForeground(backColor);
  dc.fillRectangle(ev->rect.x,ev->rect.y,ev->rect.w,ev->rect.h);

  // Control rectangle (spec section 4): the widget minus padding/border
  FXint ox=border+padleft;
  FXint oy=border+padtop;
  FXint ch=height-padtop-padbottom-(border<<1);
  FXint lineHeight=font->getFontHeight();

  // Spec section 3 [hypothesis]: circle and text line each centered by
  // integer division -- h=16 gives text one row above the circle top,
  // h=17 gives text level with it, matching both measured offsets
  FXint cx=ox;
  FXint cy=oy+(ch-CIRCLE_SIZE)/2;
  FXint tx=cx+LABEL_OFFSET;
  FXint ty=oy+(ch-lineHeight)/2;

  // Pressed: FXRadioButton sets check on press, but Windows changes
  // nothing until release (then selects this one and clears the group in
  // one step), so draw the pre-press state (oldcheck) while held. The
  // pressed look tracks the pointer the way FXRadioButton's onEnter/
  // onLeave move check away from oldcheck -- which only works when the
  // radio wasn't already selected; an already-selected one keeps the
  // pressed look for the whole press, inside or out
  FXbool enabled=isEnabled();
  FXbool held=enabled && (flags&FLAG_PRESSED);
  FXbool pressed=held && (oldcheck || check!=oldcheck);
  FXuchar shown=held ? oldcheck : check;

  // Spec section 2 colors, derived from the theme: Win2000 darkens the
  // inner top-left ring to 3DDKSHADOW, Win98 lightens the inner bottom-
  // right ring to 3DLIGHT (on Win95/2000 it equals the face)
  FXColor innerTL=(themeStyle==CYT_STYLE_2000) ? cytAverage(borderColor,shadowColor) : borderColor;
  FXColor innerBR=(themeStyle==CYT_STYLE_98) ? cytAverage(backColor,hiliteColor) : backColor;
  FXColor interior=(enabled && !pressed) ? diskColor : backColor;

  cytDrawCircle(dc,cx,cy,shadowColor,innerTL,innerBR,hiliteColor,interior);
  if(shown!=false){
    cytDrawDot(dc,cx,cy,enabled ? radioColor : shadowColor);
    }

  // Spec section 3: label at circle-left+17, mnemonic underlined below the
  // next glyph across its full advance (drawLabel's own underline)
  if(!label.empty()){
    FXint tw=labelWidth(label);
    FXint th=labelHeight(label);
    dc.setFont(font);
    if(!enabled){
      if(engrave3DStyle){
        // Engraved (Windows 2000 style): hiliteColor copy at +1,+1 under
        // the shadowColor text -- same two-pass treatment as CYTCommandButton
        dc.setForeground(hiliteColor);
        drawLabel(dc,label,hotoff,tx+1,ty+1,tw,th);
        dc.setForeground(shadowColor);
        drawLabel(dc,label,hotoff,tx,ty,tw,th);
        }
      else{
        // Flat gray, single pass (Windows 95/98 style)
        dc.setForeground(shadowColor);
        drawLabel(dc,label,hotoff,tx,ty,tw,th);
        }
      }
    else{
      dc.setForeground(textColor);
      drawLabel(dc,label,hotoff,tx,ty,tw,th);
      }

    // Spec section 4: focus rectangle one pixel outside the text, height
    // lineHeight+3 (16 for sserife.fon), dots local to the control origin
    if(enabled && hasFocus()){
      cytDrawFocusRect(dc,borderColor,ox,oy,tx-1,ty-1,tx+tw,ty+lineHeight+1);
      }
    }

  drawFrame(dc,0,0,width,height);
  return 1;
  }

}
