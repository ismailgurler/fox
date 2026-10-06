/********************************************************************************
*                                                                               *
*        Test CYTGroupBox + CYTCommandButton vs. FXGroupBox + FXButton          *
*                                                                               *
********************************************************************************/
#include "fx.h"


// Standard Win95 dialog button size (75x23 at 96 DPI, MS Sans Serif 8pt)
#define BTN_OPTS  (BUTTON_NORMAL|LAYOUT_SIDE_LEFT|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT)
#define BTN_W     75
#define BTN_H     23


// One group box holding a row of buttons side by side
template<class GROUPBOX,class BUTTON>
static GROUPBOX* buttonRow(FXComposite* column,const FXString& title,FXbool enabled){
  GROUPBOX *g=new GROUPBOX(column,title,GROUPBOX_TITLE_LEFT|FRAME_GROOVE|LAYOUT_FILL_X);
  const FXchar *labels[]={"OK","Cancel","Apply"};
  for(FXint i=0; i<3; i++){
    BUTTON *b=new BUTTON(g,labels[i],nullptr,nullptr,0,BTN_OPTS|(i==0?BUTTON_DEFAULT|BUTTON_INITIAL:BUTTON_DEFAULT),0,0,BTN_W,BTN_H);
    if(!enabled) b->disable();
    }
  if(!enabled) g->disable();
  return g;
  }


// Fill one column with the same set of group boxes, built by either class pair
template<class GROUPBOX,class BUTTON>
static void populate(FXComposite* column){
  buttonRow<GROUPBOX,BUTTON>(column,"Buttons",true);
  buttonRow<GROUPBOX,BUTTON>(column,"Disabled",false);
  GROUPBOX *g=buttonRow<GROUPBOX,BUTTON>(column,FXString::null,true);
  fxmessage("%s column: %s + %s\n",GROUPBOX::metaClass.getClassName(),g->getClassName(),g->getFirst()->getClassName());
  }


int main(int argc,char *argv[]){
  FXApp application("CYTGroupButtons","FoxTest");
  application.init(argc,argv);

  FXMainWindow *main=new FXMainWindow(&application,"CYTGroupBox + Buttons Test",nullptr,nullptr,DECOR_ALL);
  FXHorizontalFrame *contents=new FXHorizontalFrame(main,LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0,10,10,10,10,20,0);

  FXVerticalFrame *left=new FXVerticalFrame(contents,LAYOUT_FILL_Y,0,0,0,0,0,0,0,0,0,8);
  new FXLabel(left,"FXGroupBox + FXButton");
  populate<FXGroupBox,FXButton>(left);

  FXVerticalFrame *right=new FXVerticalFrame(contents,LAYOUT_FILL_Y,0,0,0,0,0,0,0,0,0,8);
  new FXLabel(right,"CYTGroupBox + CYTCommandButton");
  populate<CYTGroupBox,CYTCommandButton>(right);

  application.create();
  main->show(PLACEMENT_SCREEN);
  return application.run();
  }
