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
#ifndef BITMAPFONTDIALOG_H
#define BITMAPFONTDIALOG_H

// ControlPanel-only picker for Windows .FON/.FNT bitmap fonts: scans
// SETTINGS/bitmapfontpath (see FXFont::listBitmapFonts(), FXFont::
// defaultBitmapFontPath) for .fon files, lists the families/styles/sizes
// it finds (a single .fon can bundle several point sizes, and Windows
// ships separate files per weight/style rather than bundling those in
// one file), and shows a live preview -- mirroring FXFontDialog's own
// family/style/size/preview layout, but for bitmap fonts specifically.
//
// FXFontSelector (the shared, toolkit-wide widget behind FXFontDialog)
// now lists bitmap fonts too, alongside Xft ones, in the same merged
// list -- see FXFontSelector.cpp. This dialog remains as ControlPanel's
// own dedicated bitmap-only picker (its "Bitmap Font..." button is a
// deliberate shortcut alongside the "Choose Font..." button, not a
// second implementation of the same thing).
class BitmapFontDialog : public FXDialogBox {
  FXDECLARE(BitmapFontDialog)
protected:
  FXList      *familyList;
  FXList      *styleList;
  FXList      *sizeList;
  FXLabel     *previewLabel;
  FXFont      *previewFont;
  FXArray<FXBitmapFontEntry> entries;   // flat scan results, one per (file, FNT resource)
  FXString     fontSpec;                // "path,deci-points" of the current selection, "" if none
protected:
  BitmapFontDialog(){}
  void listFamilies();
  void listStyles();
  void listSizes();
  void updatePreview();
  FXString styleLabel(FXushort weight,FXbool italic);
private:
  BitmapFontDialog(const BitmapFontDialog&);
  BitmapFontDialog &operator=(const BitmapFontDialog&);
public:
  enum {
    ID_FAMILY=FXDialogBox::ID_LAST,
    ID_STYLE,
    ID_SIZE
    };
public:
  long onCmdFamily(FXObject*,FXSelector,void*);
  long onCmdStyle(FXObject*,FXSelector,void*);
  long onCmdSize(FXObject*,FXSelector,void*);
public:

  /// Construct dialog, scanning the given ':'-separated list of directories for .fon files
  BitmapFontDialog(FXWindow* owner,const FXString& title,const FXString& searchpath);

  /// Create server-side resources
  virtual void create();

  /// Preselect a font, given as a plain .fon path or a "path,deci-points" spec (as produced by getFontSpec())
  void setFontSpec(const FXString& spec);

  /// Return "path,deci-points" of the currently selected bitmap font, or empty if none available/selected
  FXString getFontSpec() const { return fontSpec; }

  /// True if at least one bitmap font was found on the search path
  FXbool haveFonts() const { return 0<entries.no(); }

  /// Destructor
  virtual ~BitmapFontDialog();
  };

#endif
