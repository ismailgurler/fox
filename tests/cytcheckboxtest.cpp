/********************************************************************************
*                                                                               *
*                  Test CYTCheckBox vs. stock FXCheckButton                     *
*                                                                               *
********************************************************************************/
#include "fx.h"


// Zero padding and a fixed height, so the control rectangle is exactly the
// Windows one: height 16 puts the text one row above the box (the Win98
// captures), 17 puts it level with the box (the Win95 disabled capture)
#define CHECK_OPTS (CHECKBUTTON_NORMAL|JUSTIFY_LEFT|LAYOUT_FIX_HEIGHT)


// The same set of check boxes, built by either class pair
template<class GROUPBOX,class CHECK>
static void populate(FXComposite* column){
  const FXchar *labels[]={"&Energy","&Plug","&Reset"};
  CHECK *c;
  FXint heights[]={16,17};
  for(FXint h=0; h<2; h++){
    GROUPBOX *g=new GROUPBOX(column,heights[h]==16?"Height 16":"Height 17",GROUPBOX_TITLE_LEFT|FRAME_GROOVE|LAYOUT_FILL_X);
    for(FXint i=0; i<3; i++){
      c=new CHECK(g,labels[i],nullptr,0,CHECK_OPTS,0,0,0,heights[h],0,0,0,0);
      c->setCheck(i!=1);
      }
    }

  // Disabled, with the measured Win95 capture's labels
  GROUPBOX *g=new GROUPBOX(column,"Disabled",GROUPBOX_TITLE_LEFT|FRAME_GROOVE|LAYOUT_FILL_X);
  c=new CHECK(g,"Disconnect",nullptr,0,CHECK_OPTS,0,0,0,17,0,0,0,0);
  c->disable();
  c=new CHECK(g,"Int 13 unit",nullptr,0,CHECK_OPTS,0,0,0,17,0,0,0,0);
  c->setCheck(true);
  c->disable();

  // Third state (MAYBE), unmeasured
  g=new GROUPBOX(column,"Third state",GROUPBOX_TITLE_LEFT|FRAME_GROOVE|LAYOUT_FILL_X);
  c=new CHECK(g,"&Indeterminate",nullptr,0,CHECK_OPTS,0,0,0,16,0,0,0,0);
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
