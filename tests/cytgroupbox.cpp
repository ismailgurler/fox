/********************************************************************************
*                                                                               *
*                    Test CYTGroupBox vs. stock FXGroupBox                      *
*                                                                               *
********************************************************************************/
#include "fx.h"


// Disabled-label override: CYT widgets take an explicit setting; stock FOX
// widgets have none, so their rows show FOX's own disabled look
static void engraveStyle(FXWindow*,FXuchar){}
static void engraveStyle(CYTGroupBox* w,FXuchar s){ w->setEngrave3DStyle(s); }
static void engraveStyle(CYTCheckBox* w,FXuchar s){ w->setEngrave3DStyle(s); }
static void engraveStyle(CYTRadioButton* w,FXuchar s){ w->setEngrave3DStyle(s); }


// Fill one column with the same set of group boxes, built by either class set.
// Check and radio controls get zero padding, so the CYT ones take exactly the
// Windows control rectangle (16px tall); the stock ones get the same, to match.
// Both are 160px wide, more than the label needs, so the empty strip right of
// the text can be clicked: the whole control rectangle is the hit area.
// The disabled rows: one follows the theme (SETTINGS/engrave3dstyle), two
// force the disabled-label style on the box title and both controls.
template<class GROUPBOX,class CHECK,class RADIO>
static void populate(FXComposite* column){
  GROUPBOX *g;
  CHECK *c=nullptr;
  RADIO *r=nullptr;
  const FXchar *titles[]={"Labeled, title left","Disabled (theme)","Disabled, engraved","Disabled, flat"};
  FXuchar styles[]={maybe,maybe,true,false};
  for(FXint i=0; i<4; i++){
    g=new GROUPBOX(column,titles[i],GROUPBOX_TITLE_LEFT|FRAME_GROOVE|LAYOUT_FILL_X);
    c=new CHECK(g,"Check button",nullptr,0,JUSTIFY_LEFT|ICON_BEFORE_TEXT|LAYOUT_FIX_WIDTH,0,0,160,0,0,0,0,0);
    r=new RADIO(g,"Radio button",nullptr,0,JUSTIFY_LEFT|ICON_BEFORE_TEXT|LAYOUT_FIX_WIDTH,0,0,160,0,0,0,0,0);
    if(i>0){
      g->disable();
      c->disable();
      r->disable();
      engraveStyle(g,styles[i]);
      engraveStyle(c,styles[i]);
      engraveStyle(r,styles[i]);
      }
    }
  g=new GROUPBOX(column,"Title center",GROUPBOX_TITLE_CENTER|FRAME_GROOVE|LAYOUT_FILL_X);
  new FXLabel(g,"Centered title");
  g=new GROUPBOX(column,"Title right",GROUPBOX_TITLE_RIGHT|FRAME_GROOVE|LAYOUT_FILL_X);
  new FXLabel(g,"Right-aligned title");
  g=new GROUPBOX(column,FXString::null,GROUPBOX_NORMAL|FRAME_GROOVE|LAYOUT_FILL_X);
  new FXLabel(g,"No label: frame should be unbroken");
  fxmessage("%s column: %s + %s + %s\n",GROUPBOX::metaClass.getClassName(),g->getClassName(),c->getClassName(),r->getClassName());
  }


int main(int argc,char *argv[]){
  FXApp application("CYTGroupBox","FoxTest");
  application.init(argc,argv);

  FXMainWindow *main=new FXMainWindow(&application,"CYTGroupBox Test",nullptr,nullptr,DECOR_ALL);
  FXHorizontalFrame *contents=new FXHorizontalFrame(main,LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0,10,10,10,10,20,0);

  FXVerticalFrame *left=new FXVerticalFrame(contents,LAYOUT_FILL_X|LAYOUT_FILL_Y);
  new FXLabel(left,"Stock FXGroupBox + FX controls");
  populate<FXGroupBox,FXCheckButton,FXRadioButton>(left);

  FXVerticalFrame *right=new FXVerticalFrame(contents,LAYOUT_FILL_X|LAYOUT_FILL_Y);
  new FXLabel(right,"CYTGroupBox + CYT controls");
  populate<CYTGroupBox,CYTCheckBox,CYTRadioButton>(right);

  application.create();
  main->show(PLACEMENT_SCREEN);
  return application.run();
  }
