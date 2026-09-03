/********************************************************************************
*                                                                               *
*             B i t m a p   F o n t   S e l e c t i o n   D i a l o g          *
*                                                                               *
*********************************************************************************
* This library is free software; you can redistribute it and/or modify          *
* it under the terms of the GNU Lesser General Public License as published by   *
* the Free Software Foundation; either version 3 of the License, or             *
* (at your option) any later version.                                           *
********************************************************************************/
#include <xincs.h>
#include <fx.h>
#include "BitmapFontDialog.h"

/*
  Notes:
  - Search path is a plain compile-time default for now (see
    BITMAPFONTPATH below) rather than a registry setting/UI field --
    matching FXIconCache::defaultIconPath's "~" and PATHLISTSEP-separated
    convention, but not (yet) exposed anywhere to change it. Worth adding
    as a real ControlPanel General-tab setting (mirroring "Icon Search
    Path") if this dialog turns out to be useful -- noted in PLAN.md.
  - Family/style/size lists are built from a flat scan (fxListBitmapFonts)
    each time the dialog opens; for the handful of .fon files anyone is
    likely to have installed this is instant, so no caching.
*/

#define BITMAPFONTPATH "/usr/local/share/fonts:~/.local/share/fonts"

/*******************************************************************************/

// Map
FXDEFMAP(BitmapFontDialog) BitmapFontDialogMap[]={
  FXMAPFUNC(SEL_COMMAND,BitmapFontDialog::ID_FAMILY,BitmapFontDialog::onCmdFamily),
  FXMAPFUNC(SEL_COMMAND,BitmapFontDialog::ID_STYLE,BitmapFontDialog::onCmdStyle),
  FXMAPFUNC(SEL_COMMAND,BitmapFontDialog::ID_SIZE,BitmapFontDialog::onCmdSize),
  };


// Object implementation
FXIMPLEMENT(BitmapFontDialog,FXDialogBox,BitmapFontDialogMap,ARRAYNUMBER(BitmapFontDialogMap))


// Construct dialog
BitmapFontDialog::BitmapFontDialog(FXWindow* owner,const FXString& title,const FXString& searchpath):
  FXDialogBox(owner,title,DECOR_TITLE|DECOR_BORDER|DECOR_RESIZE|DECOR_CLOSE,0,0,540,380,0,0,0,0),
  familyList(nullptr),styleList(nullptr),sizeList(nullptr),previewLabel(nullptr),previewFont(nullptr){

  fxListBitmapFonts(searchpath.empty()?FXString(BITMAPFONTPATH):searchpath,entries);

  FXVerticalFrame* main=new FXVerticalFrame(this,LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0,10,10,10,10);

  FXHorizontalFrame* listsframe=new FXHorizontalFrame(main,LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0,0,0,0,0);

  FXVerticalFrame* famframe=new FXVerticalFrame(listsframe,LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0,0,0,0,0,0,4);
  new FXLabel(famframe,tr("Family"));
  familyList=new FXList(famframe,this,ID_FAMILY,LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN|FRAME_THICK|LIST_BROWSESELECT);
  familyList->setSortFunc(FXList::ascending);

  new FXSeparator(listsframe,SEPARATOR_NONE|LAYOUT_FIX_WIDTH,10,0);

  FXVerticalFrame* styframe=new FXVerticalFrame(listsframe,LAYOUT_FILL_Y|LAYOUT_FIX_WIDTH,0,0,140,0,0,0,0,0,0,4);
  new FXLabel(styframe,tr("Style"));
  styleList=new FXList(styframe,this,ID_STYLE,LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN|FRAME_THICK|LIST_BROWSESELECT);
  styleList->setSortFunc(FXList::ascending);

  new FXSeparator(listsframe,SEPARATOR_NONE|LAYOUT_FIX_WIDTH,10,0);

  FXVerticalFrame* sizframe=new FXVerticalFrame(listsframe,LAYOUT_FILL_Y|LAYOUT_FIX_WIDTH,0,0,80,0,0,0,0,0,0,4);
  new FXLabel(sizframe,tr("Size"));
  sizeList=new FXList(sizframe,this,ID_SIZE,LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN|FRAME_THICK|LIST_BROWSESELECT);

  new FXSeparator(main,SEPARATOR_GROOVE|LAYOUT_FILL_X);

  FXGroupBox* previewgroup=new FXGroupBox(main,tr("Preview"),FRAME_GROOVE|LAYOUT_FILL_X,0,0,0,0,10,10,10,10);
  previewLabel=new FXLabel(previewgroup,tr("The quick brown fox jumps over the lazy dog 0123456789"),nullptr,JUSTIFY_LEFT|LAYOUT_FILL_X,0,0,0,60);

  new FXSeparator(main,SEPARATOR_GROOVE|LAYOUT_FILL_X);

  FXHorizontalFrame* btns=new FXHorizontalFrame(main,LAYOUT_FILL_X,0,0,0,0,0,0,0,0);
  new FXFrame(btns,LAYOUT_FILL_X);
  new FXButton(btns,tr("&Cancel"),nullptr,this,FXDialogBox::ID_CANCEL,FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT,0,0,0,0,20,20);
  new FXButton(btns,tr("&OK"),nullptr,this,FXDialogBox::ID_ACCEPT,FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT,0,0,0,0,20,20);

  listFamilies();
  }


// Create server-side resources
void BitmapFontDialog::create(){
  FXDialogBox::create();
  show(PLACEMENT_OWNER);
  }


// Human-readable label for a (weight,italic) combination
FXString BitmapFontDialog::styleLabel(FXushort weight,FXbool italic){
  // Not static: tr() needs an object to resolve translations against.
  FXbool bold=(weight>=FXFont::DemiBold);
  if(bold && italic) return tr("Bold Italic");
  if(bold) return tr("Bold");
  if(italic) return tr("Italic");
  return tr("Regular");
  }


// Populate family list with unique family names found by the scan
void BitmapFontDialog::listFamilies(){
  familyList->clearItems();
  for(FXival i=0; i<entries.no(); i++){
    if(familyList->findItem(entries[i].family)<0){
      familyList->appendItem(entries[i].family);
      }
    }
  familyList->sortItems();
  if(0<familyList->getNumItems()){
    familyList->setCurrentItem(0,true);
    }
  listStyles();
  }


// Populate style list with the (weight,italic) combinations available
// for the currently selected family, packed into each item's data as
// (weight<<1)|italic.
void BitmapFontDialog::listStyles(){
  styleList->clearItems();
  if(0<=familyList->getCurrentItem()){
    FXString family=familyList->getItemText(familyList->getCurrentItem());
    for(FXival i=0; i<entries.no(); i++){
      if(entries[i].family!=family) continue;
      FXival tag=(((FXival)entries[i].weight)<<1)|(entries[i].italic?1:0);
      if(styleList->findItemByData((void*)tag)<0){
        styleList->appendItem(styleLabel(entries[i].weight,entries[i].italic),nullptr,(void*)tag);
        }
      }
    styleList->sortItems();
    }
  if(0<styleList->getNumItems()){
    styleList->setCurrentItem(0,true);
    }
  listSizes();
  }


// Populate size list (whole points) available for the currently selected family+style
void BitmapFontDialog::listSizes(){
  sizeList->clearItems();
  if(0<=familyList->getCurrentItem() && 0<=styleList->getCurrentItem()){
    FXString family=familyList->getItemText(familyList->getCurrentItem());
    FXival tag=(FXival)styleList->getItemData(styleList->getCurrentItem());
    // Collect the distinct point sizes first and sort them *numerically* --
    // FXList::ascending sorts by item text, which would put "10" before
    // "8" (lexicographic, not numeric) if applied here.
    FXushort sizes[64];
    FXint nsizes=0;
    for(FXival i=0; i<entries.no() && nsizes<(FXint)ARRAYNUMBER(sizes); i++){
      if(entries[i].family!=family) continue;
      FXival etag=(((FXival)entries[i].weight)<<1)|(entries[i].italic?1:0);
      if(etag!=tag) continue;
      FXushort pts=entries[i].points;
      FXint j=0;
      while(j<nsizes && sizes[j]!=pts) j++;
      if(j==nsizes){
        while(j>0 && pts<sizes[j-1]){ sizes[j]=sizes[j-1]; j--; }
        sizes[j]=pts;
        nsizes++;
        }
      }
    for(FXint i=0; i<nsizes; i++){
      sizeList->appendItem(FXString::value(sizes[i]),nullptr,(void*)(FXival)sizes[i]);
      }
    }
  if(0<sizeList->getNumItems()){
    sizeList->setCurrentItem(0,true);
    }
  updatePreview();
  }


// Rebuild fontSpec from the current family+style+size selection, and
// apply it to the preview label.
void BitmapFontDialog::updatePreview(){
  FXFont* oldfont=previewFont;
  previewFont=nullptr;
  fontSpec=FXString::null;
  if(0<=familyList->getCurrentItem() && 0<=styleList->getCurrentItem() && 0<=sizeList->getCurrentItem()){
    FXString family=familyList->getItemText(familyList->getCurrentItem());
    FXival styletag=(FXival)styleList->getItemData(styleList->getCurrentItem());
    FXival points=(FXival)sizeList->getItemData(sizeList->getCurrentItem());
    for(FXival i=0; i<entries.no(); i++){
      if(entries[i].family!=family) continue;
      FXival etag=(((FXival)entries[i].weight)<<1)|(entries[i].italic?1:0);
      if(etag!=styletag || entries[i].points!=points) continue;
      fontSpec=entries[i].path+","+FXString::value((FXint)(points*10));   // deci-points -- see BitmapFontDialog.h
      break;
      }
    }
  if(!fontSpec.empty()){
    previewFont=new FXFont(getApp(),fontSpec);
    previewFont->create();
    }
  previewLabel->setFont(previewFont?previewFont:getApp()->getNormalFont());
  delete oldfont;
  }


// Preselect a font already known to be a bitmap font (a plain path, or a
// "path,deci-points" spec as returned by getFontSpec())
void BitmapFontDialog::setFontSpec(const FXString& spec){
  FXString path=spec.section(',',0);
  FXint findex=-1;
  for(FXival i=0; i<entries.no() && findex<0; i++){
    if(entries[i].path==path) findex=familyList->findItem(entries[i].family);
    }
  if(0<=findex){
    familyList->setCurrentItem(findex,true);
    familyList->makeItemVisible(findex);
    listStyles();
    }
  }


// Family selection changed
long BitmapFontDialog::onCmdFamily(FXObject*,FXSelector,void*){
  listStyles();
  return 1;
  }


// Style selection changed
long BitmapFontDialog::onCmdStyle(FXObject*,FXSelector,void*){
  listSizes();
  return 1;
  }


// Size selection changed
long BitmapFontDialog::onCmdSize(FXObject*,FXSelector,void*){
  updatePreview();
  return 1;
  }


// Destructor
BitmapFontDialog::~BitmapFontDialog(){
  delete previewFont;
  familyList=(FXList*)-1L;
  styleList=(FXList*)-1L;
  sizeList=(FXList*)-1L;
  previewLabel=(FXLabel*)-1L;
  }
