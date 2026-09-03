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

// Mirrors FX::FXBitmapFontEntry and FX::fxListBitmapFonts() (lib/xfntface.h,
// lib/FXFont.cpp) exactly -- name, namespace and layout. Both are internal
// to libFOX (not an installed header), so they're re-declared here rather
// than shared; this works because name mangling only encodes the
// namespace-qualified name FX::FXBitmapFontEntry, not its member layout,
// so this links against the real symbols in libFOX-1.7.a even though the
// two declarations live in unrelated translation units (the same trick
// used by the standalone test harness that first validated
// fxListBitmapFonts() -- see PLAN.md, Phase 4). Keeping this dialog off
// the public API on purpose (see class comment below), so duplicating one
// small struct + one extern declaration is cheaper than promoting
// library-internal types to public headers.
namespace FX {
struct FXBitmapFontEntry {
  FXString path;
  FXString family;
  FXushort weight;
  FXbool   italic;
  FXushort points;
  };
extern FXbool fxListBitmapFonts(const FXString& searchpath,FXArray<FXBitmapFontEntry>& out);
}
using FX::FXBitmapFontEntry;

// ControlPanel-only picker for Windows .FON/.FNT bitmap fonts: scans a
// configured search path (see BitmapFontDialog.cpp) for .fon files, lists
// the families/styles/sizes it finds (a single .fon can bundle several
// point sizes, and Windows ships separate files per weight/style rather
// than bundling those in one file), and shows a live preview -- mirroring
// FXFontDialog's own family/style/size/preview layout, but for bitmap
// fonts specifically.
//
// Deliberately NOT folded into the shared FXFontSelector widget (that was
// the original plan -- see PLAN.md, Phase 4) since that's toolkit-wide,
// shared code every FOX app depends on; this one-off dialog is much lower
// risk and keeps the blast radius inside controlpanel/.
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
