/********************************************************************************
*                                                                               *
*                  Test CYTCheckBox vs. stock FXCheckButton                     *
*                                                                               *
********************************************************************************/
#include "fx.h"


// Zero padding and a fixed height, so the control rectangle is exactly the
// Windows one: height 16 puts the text one row above the box (the Win98
// captures), 17 puts it level with the box (the Win95 disabled capture)
#define CHECK_OPTS (CHECKBUTTON_NORMAL|JUSTIFY_LEFT|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT)

// Wider than the label needs, so the empty strip right of the text can be
// clicked: the whole control rectangle is the hit area
#define CHECK_W 160


// Disabled-label override: CYT widgets take an explicit setting; stock FOX
// widgets have none, so their rows show FOX's own disabled look
static void engraveStyle(FXWindow*,FXuchar){}
static void engraveStyle(CYTGroupBox* w,FXuchar s){ w->setEngrave3DStyle(s); }
static void engraveStyle(CYTCheckBox* w,FXuchar s){ w->setEngrave3DStyle(s); }


// A disabled group with the measured Win95 capture's labels; style applies
// to the box title and both check boxes (maybe = follow the theme)
template<class GROUPBOX,class CHECK>
static void disabledPair(FXComposite* column,const FXString& title,FXuchar style){
  GROUPBOX *g=new GROUPBOX(column,title,GROUPBOX_TITLE_LEFT|FRAME_GROOVE|LAYOUT_FILL_X);
  g->disable();
  engraveStyle(g,style);
  CHECK *c=new CHECK(g,"Disconnect",nullptr,0,CHECK_OPTS,0,0,CHECK_W,17,0,0,0,0);
  c->disable();
  engraveStyle(c,style);
  c=new CHECK(g,"Int 13 unit",nullptr,0,CHECK_OPTS,0,0,CHECK_W,17,0,0,0,0);
  c->setCheck(true);
  c->disable();
  engraveStyle(c,style);
  }


// The same set of check boxes, built by either class pair
template<class GROUPBOX,class CHECK>
static void populate(FXComposite* column){
  const FXchar *labels[]={"&Energy","&Plug","&Reset"};
  CHECK *c;
  FXint heights[]={16,17};
  for(FXint h=0; h<2; h++){
    GROUPBOX *g=new GROUPBOX(column,heights[h]==16?"Height 16":"Height 17",GROUPBOX_TITLE_LEFT|FRAME_GROOVE|LAYOUT_FILL_X);
    for(FXint i=0; i<3; i++){
      c=new CHECK(g,labels[i],nullptr,0,CHECK_OPTS,0,0,CHECK_W,heights[h],0,0,0,0);
      c->setCheck(i!=1);
      }
    }

  // Disabled: following the theme, then forced engraved and forced flat
  disabledPair<GROUPBOX,CHECK>(column,"Disabled (theme)",maybe);
  disabledPair<GROUPBOX,CHECK>(column,"Disabled, engraved",true);
  disabledPair<GROUPBOX,CHECK>(column,"Disabled, flat",false);

  // Third state (MAYBE), unmeasured
  GROUPBOX *g=new GROUPBOX(column,"Third state",GROUPBOX_TITLE_LEFT|FRAME_GROOVE|LAYOUT_FILL_X);
  c=new CHECK(g,"&Indeterminate",nullptr,0,CHECK_OPTS,0,0,CHECK_W,16,0,0,0,0);
  c->setCheck(maybe);
  fxmessage("%s column: %s + %s\n",CHECK::metaClass.getClassName(),g->getClassName(),c->getClassName());
  }


int main(int argc,char *argv[]){
  FXApp application("CYTCheckBox","FoxTest");
  application.init(argc,argv);

  FXMainWindow *main=new FXMainWindow(&application,"CYTCheckBox Test",nullptr,nullptr,DECOR_ALL);
  FXHorizontalFrame *contents=new FXHorizontalFrame(main,LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0,10,10,10,10,20,0);

  FXVerticalFrame *left=new FXVerticalFrame(contents,LAYOUT_FILL_Y,0,0,0,0,0,0,0,0,0,8);
  new FXLabel(left,"FXGroupBox + FXCheckButton");
  populate<FXGroupBox,FXCheckButton>(left);

  FXVerticalFrame *right=new FXVerticalFrame(contents,LAYOUT_FILL_Y,0,0,0,0,0,0,0,0,0,8);
  new FXLabel(right,"CYTGroupBox + CYTCheckBox");
  populate<CYTGroupBox,CYTCheckBox>(right);

  application.create();
  main->show(PLACEMENT_SCREEN);
  return application.run();
  }
