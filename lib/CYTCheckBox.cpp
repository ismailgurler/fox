/********************************************************************************
*                                                                               *
*                  C Y T C h e c k B o x   -   W i n 9 5   C h e c k b o x      *
*                                                                               *
*********************************************************************************
* Pixel-accurate replica of the classic (pre-uxtheme) Windows 95/98/2000       *
* check box, per the "Checkbox control" brief. The box, tick, label offset    *
* and flat disabled label below are taken directly from that brief's          *
* measured tables; the control rectangle, pressed look and MAYBE state are    *
* its stated assumptions.                                                      *
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
#include "FXCheckButton.h"
#include "CYTCheckBox.h"

using namespace FX;

namespace FX {

// Map -- onPaint is overridden; the press/release messages are routed
// through onPressState, which runs FXCheckButton's own handler unchanged
// and only adds a repaint. Every other FXCheckButton behavior is inherited.
FXDEFMAP(CYTCheckBox) CYTCheckBoxMap[]={
  FXMAPFUNC(SEL_PAINT,0,CYTCheckBox::onPaint),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,CYTCheckBox::onPressState),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,CYTCheckBox::onPressState),
  FXMAPFUNC(SEL_UNGRABBED,0,CYTCheckBox::onPressState),
  FXMAPFUNC(SEL_KEYPRESS,0,CYTCheckBox::onPressState),
  FXMAPFUNC(SEL_KEYRELEASE,0,CYTCheckBox::onPressState),
  FXMAPFUNC(SEL_KEYPRESS,FXWindow::ID_HOTKEY,CYTCheckBox::onPressState),
  FXMAPFUNC(SEL_KEYRELEASE,FXWindow::ID_HOTKEY,CYTCheckBox::onPressState),
  };


// Object implementation
FXIMPLEMENT(CYTCheckBox,FXCheckButton,CYTCheckBoxMap,ARRAYNUMBER(CYTCheckBoxMap))


// Box size, and the label's offset from the box's left edge (measured)
#define BOX_SIZE      13
#define LABEL_OFFSET  18


// The 13x13 box, local (col,row), identical on Win95, Win98 and Win2000
// (only the colors differ). G outer top-left, K inner top-left, L inner
// bottom-right, W outer bottom-right, i the 9x9 interior.
static const FXchar boxBitmap[BOX_SIZE][BOX_SIZE+1]={
  "GGGGGGGGGGGGW",
  "GKKKKKKKKKKLW",
  "GKiiiiiiiiiLW",
  "GKiiiiiiiiiLW",
  "GKiiiiiiiiiLW",
  "GKiiiiiiiiiLW",
  "GKiiiiiiiiiLW",
  "GKiiiiiiiiiLW",
  "GKiiiiiiiiiLW",
  "GKiiiiiiiiiLW",
  "GKiiiiiiiiiLW",
  "GLLLLLLLLLLLW",
  "WWWWWWWWWWWWW",
  };


// The tick, box-local (col,row) as horizontal runs {row,col,length}:
// row 3: 9 | row 4: 8-9 | row 5: 3,7-9 | row 6: 3-4,6-8 | row 7: 3-7 |
// row 8: 4-6 | row 9: 5
static const FXuchar tickRuns[][3]={
  {3,9,1},{4,8,2},{5,3,1},{5,7,3},{6,3,2},{6,6,3},{7,3,5},{8,4,3},{9,5,1}
  };


// Per-channel average, rounded down -- how Windows derives COLOR_3DLIGHT
// (face+highlight) and COLOR_3DDKSHADOW (windowframe+shadow); same formula
// as CYTCommandButton's
static FXColor cytAverage(FXColor a,FXColor b){
  return FXRGB((FXREDVAL(a)+FXREDVAL(b))/2,(FXGREENVAL(a)+FXGREENVAL(b))/2,(FXBLUEVAL(a)+FXBLUEVAL(b))/2);
  }


// Construct check box; same signature as FXCheckButton. Border and
// disabled-text styles are read once here from the registry, same keys
// and "takes effect next app launch" convention as CYTCommandButton.
CYTCheckBox::CYTCheckBox(FXComposite* p,const FXString& text,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb):
  FXCheckButton(p,text,tgt,sel,opts,x,y,w,h,pl,pr,pt,pb),
  engrave3DStyle(getApp()->reg().readBoolEntry("SETTINGS","engrave3dstyle",false)),
  themeStyle(FXCLAMP(CYT_STYLE_95,getApp()->reg().readIntEntry("SETTINGS","themestyle",CYT_STYLE_95),CYT_STYLE_2000)){
  }


/*******************************************************************************/

// Get default width -- text starts at 18 and the focus rectangle's right
// edge is one pixel past the text, at 18+textWidth
FXint CYTCheckBox::getDefaultWidth(){
  FXint w=label.empty() ? BOX_SIZE : LABEL_OFFSET+1+labelWidth(label);
  return padleft+padright+w+(border<<1);
  }


// Get default height -- 16 for sserife.fon's 13px line (the focus
// rectangle needs lineHeight+3 rows), never less than the box
FXint CYTCheckBox::getDefaultHeight(){
  FXint h=FXMAX(font->getFontHeight()+3,BOX_SIZE);
  return padtop+padbottom+h+(border<<1);
  }


/*******************************************************************************/

// Press/release (mouse, space, hotkey, lost grab): let FXCheckButton
// handle it exactly as before, then repaint if FLAG_PRESSED changed --
// FXCheckButton only repaints when `check` changes, and on release it
// already holds its final value, so the pressed look would stay on screen
long CYTCheckBox::onPressState(FXObject* sender,FXSelector sel,void* ptr){
  FXuint was=flags&FLAG_PRESSED;
  long res=FXCheckButton::handle(sender,sel,ptr);
  if((flags&FLAG_PRESSED)!=was) update();
  return res;
  }


/*******************************************************************************/

// Draw the box bitmap at (x,y), one color per key letter, merging
// horizontal runs of the same key into one fill
static void cytDrawBox(FXDCWindow& dc,FXint x,FXint y,FXColor outerTL,FXColor innerTL,FXColor innerBR,FXColor outerBR,FXColor interior){
  FXint row,col,end;
  FXchar key;
  for(row=0; row<BOX_SIZE; ++row){
    for(col=0; col<BOX_SIZE; col=end){
      key=boxBitmap[row][col];
      for(end=col+1; end<BOX_SIZE && boxBitmap[row][end]==key; ++end){}
      switch(key){
        case 'G': dc.setForeground(outerTL); break;
        case 'K': dc.setForeground(innerTL); break;
        case 'L': dc.setForeground(innerBR); break;
        case 'W': dc.setForeground(outerBR); break;
        default:  dc.setForeground(interior); break;
        }
      dc.fillRectangle(x+col,y+row,end-col,1);
      }
    }
  }


// Draw the tick at box origin (x,y)
static void cytDrawTick(FXDCWindow& dc,FXint x,FXint y,FXColor color){
  dc.setForeground(color);
  for(FXuint i=0; i<ARRAYNUMBER(tickRuns); ++i){
    dc.fillRectangle(x+tickRuns[i][1],y+tickRuns[i][0],tickRuns[i][2],1);
    }
  }


// Draw the dotted focus rectangle: perimeter of (x0,y0)-(x1,y1) inclusive,
// a dot wherever x+y is odd in coordinates local to the control origin
// (ox,oy) -- so whether the corner gets a dot depends on where the
// rectangle sits in the control, never hard-coded
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
long CYTCheckBox::onPaint(FXObject*,FXSelector,void* ptr){
  FXEvent *ev=(FXEvent*)ptr;
  FXDCWindow dc(this,ev);

  dc.setForeground(backColor);
  dc.fillRectangle(ev->rect.x,ev->rect.y,ev->rect.w,ev->rect.h);

  // Control rectangle: the widget minus padding/border
  FXint ox=border+padleft;
  FXint oy=border+padtop;
  FXint ch=height-padtop-padbottom-(border<<1);
  FXint lineHeight=font->getFontHeight();

  // [hypothesis] box and text line each centered by integer division --
  // a 16px control gives box row 2, text row 1 (text = box-1, focus
  // rectangle flush with the top); 17px gives text level with the box
  // (box+0, as in the measured disabled capture)
  FXint bx=ox;
  FXint by=oy+(ch-BOX_SIZE+1)/2;
  FXint tx=bx+LABEL_OFFSET;
  FXint ty=oy+(ch-lineHeight)/2;

  // Pressed (pointer inside while held): FXCheckButton has already
  // toggled check away from oldcheck, but Windows shows the old state
  // until release
  FXbool enabled=isEnabled();
  FXbool pressed=enabled && (flags&FLAG_PRESSED) && (check!=oldcheck);
  FXuchar shown=(enabled && (flags&FLAG_PRESSED)) ? oldcheck : check;

  // Colors, derived from the theme: Win2000 darkens the inner top-left
  // edge to 3DDKSHADOW, Win98 lightens the inner bottom-right ring to
  // 3DLIGHT (on Win95/2000 it equals the face)
  FXColor innerTL=(themeStyle==CYT_STYLE_2000) ? cytAverage(borderColor,shadowColor) : borderColor;
  FXColor innerBR=(themeStyle==CYT_STYLE_98) ? cytAverage(backColor,hiliteColor) : backColor;
  FXColor interior=(enabled && !pressed && shown!=maybe) ? boxColor : backColor;

  cytDrawBox(dc,bx,by,shadowColor,innerTL,innerBR,hiliteColor,interior);
  if(shown!=false){
    cytDrawTick(dc,bx,by,(enabled && shown!=maybe) ? checkColor : shadowColor);
    }

  // Label at box-left+18, mnemonic underlined in row 12 of the glyph cell
  // across the character's advance (drawLabel's own underline)
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
        // Flat gray, single pass -- the measured Win95 disabled label
        dc.setForeground(shadowColor);
        drawLabel(dc,label,hotoff,tx,ty,tw,th);
        }
      }
    else{
      dc.setForeground(textColor);
      drawLabel(dc,label,hotoff,tx,ty,tw,th);
      }

    // Focus rectangle one pixel outside the text: width textWidth+2,
    // height lineHeight+3 (16 for sserife.fon), dots local to the control
    if(enabled && hasFocus()){
      cytDrawFocusRect(dc,borderColor,ox,oy,tx-1,ty-1,tx+tw,ty+lineHeight+1);
      }
    }

  drawFrame(dc,0,0,width,height);
  return 1;
  }

}
