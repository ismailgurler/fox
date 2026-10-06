/********************************************************************************
*                                                                               *
*                   Test CYTCommandButton vs. stock FXButton                    *
*                                                                               *
********************************************************************************/
#include "fx.h"


// Standard Win95 dialog button size (75x23 at 96 DPI, MS Sans Serif 8pt)
#define BTN_OPTS  (BUTTON_NORMAL|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT)
#define BTN_W     75
#define BTN_H     23


// Fill one column with the same set of buttons, built by either class
template<class BUTTON>
static void populate(FXComposite* column){
  BUTTON *b;
  new BUTTON(column,"Normal",nullptr,nullptr,0,BTN_OPTS,0,0,BTN_W,BTN_H);
  new BUTTON(column,"Default",nullptr,nullptr,0,BTN_OPTS|BUTTON_DEFAULT|BUTTON_INITIAL,0,0,BTN_W,BTN_H);
  b=new BUTTON(column,"Pressed",nullptr,nullptr,0,BTN_OPTS,0,0,BTN_W,BTN_H);
  b->setState(STATE_DOWN);
  b=new BUTTON(column,"Disabled",nullptr,nullptr,0,BTN_OPTS,0,0,BTN_W,BTN_H);
  b->disable();
  FXHorizontalFrame *row=new FXHorizontalFrame(column,LAYOUT_FILL_X,0,0,0,0,0,0,0,0,6,0);
  new BUTTON(row,"OK",nullptr,nullptr,0,BTN_OPTS|BUTTON_DEFAULT,0,0,BTN_W,BTN_H);
  b=new BUTTON(row,"Cancel",nullptr,nullptr,0,BTN_OPTS|BUTTON_DEFAULT,0,0,BTN_W,BTN_H);
  fxmessage("%s column: %s\n",BUTTON::metaClass.getClassName(),b->getClassName());
  }


int main(int argc,char *argv[]){
  FXApp application("CYTCommandButton","FoxTest");
  application.init(argc,argv);

  FXMainWindow *main=new FXMainWindow(&application,"CYTCommandButton Test",nullptr,nullptr,DECOR_ALL);
  FXHorizontalFrame *contents=new FXHorizontalFrame(main,LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0,10,10,10,10,30,0);

  FXVerticalFrame *left=new FXVerticalFrame(contents,LAYOUT_FILL_Y,0,0,0,0,0,0,0,0,0,8);
  new FXLabel(left,"Stock FXButton");
  populate<FXButton>(left);

  FXVerticalFrame *right=new FXVerticalFrame(contents,LAYOUT_FILL_Y,0,0,0,0,0,0,0,0,0,8);
  new FXLabel(right,"CYTCommandButton");
  populate<CYTCommandButton>(right);

  application.create();
  main->show(PLACEMENT_SCREEN);
  return application.run();
  }
