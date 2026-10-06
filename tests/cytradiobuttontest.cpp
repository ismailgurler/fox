/********************************************************************************
*                                                                               *
*                Test CYTRadioButton vs. stock FXRadioButton                    *
*                                                                               *
********************************************************************************/
#include "fx.h"


// Zero padding and a fixed height, so the control rectangle is exactly the
// Windows one (spec section 4): heights 16 and 17 exercise the 0/-1 text
// offset rule, and the focus rectangle's corner-dot parity
#define RADIO_OPTS (RADIOBUTTON_NORMAL|JUSTIFY_LEFT|LAYOUT_FIX_HEIGHT)


// One group box of mutually exclusive radios (linked through one data
// target), plus a disabled pair, built by either class pair
template<class GROUPBOX,class RADIO>
static void populate(FXComposite* column,FXDataTarget* h16,FXDataTarget* h17){
  const FXchar *labels[]={"&Multiple","&Single","Te&xtured","Soli&d"};
  GROUPBOX *g=new GROUPBOX(column,"Height 16",GROUPBOX_TITLE_LEFT|FRAME_GROOVE|LAYOUT_FILL_X);
  for(FXint i=0; i<4; i++){
    new RADIO(g,labels[i],h16,FXDataTarget::ID_OPTION+i,RADIO_OPTS,0,0,0,16,0,0,0,0);
    }
  g=new GROUPBOX(column,"Height 17",GROUPBOX_TITLE_LEFT|FRAME_GROOVE|LAYOUT_FILL_X);
  for(FXint i=0; i<4; i++){
    new RADIO(g,labels[i],h17,FXDataTarget::ID_OPTION+i,RADIO_OPTS,0,0,0,17,0,0,0,0);
    }
  g=new GROUPBOX(column,"Disabled",GROUPBOX_TITLE_LEFT|FRAME_GROOVE|LAYOUT_FILL_X);
  RADIO *r=new RADIO(g,"&Checked",nullptr,0,RADIO_OPTS,0,0,0,16,0,0,0,0);
  r->setCheck(true);
  r->disable();
  r=new RADIO(g,"&Unchecked",nullptr,0,RADIO_OPTS,0,0,0,16,0,0,0,0);
  r->disable();
  fxmessage("%s column: %s + %s\n",RADIO::metaClass.getClassName(),g->getClassName(),r->getClassName());
  }


int main(int argc,char *argv[]){
  FXApp application("CYTRadioButton","FoxTest");
  application.init(argc,argv);

  // Each group gets its own selection, starting on the second option
  FXint choice[4]={1,1,1,1};
  FXDataTarget stock16(choice[0]),stock17(choice[1]),cyt16(choice[2]),cyt17(choice[3]);

  FXMainWindow *main=new FXMainWindow(&application,"CYTRadioButton Test",nullptr,nullptr,DECOR_ALL);
  FXHorizontalFrame *contents=new FXHorizontalFrame(main,LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0,10,10,10,10,20,0);

  FXVerticalFrame *left=new FXVerticalFrame(contents,LAYOUT_FILL_Y,0,0,0,0,0,0,0,0,0,8);
  new FXLabel(left,"FXGroupBox + FXRadioButton");
  populate<FXGroupBox,FXRadioButton>(left,&stock16,&stock17);

  FXVerticalFrame *right=new FXVerticalFrame(contents,LAYOUT_FILL_Y,0,0,0,0,0,0,0,0,0,8);
  new FXLabel(right,"CYTGroupBox + CYTRadioButton");
  populate<CYTGroupBox,CYTRadioButton>(right,&cyt16,&cyt17);

  application.create();
  main->show(PLACEMENT_SCREEN);
  return application.run();
  }
