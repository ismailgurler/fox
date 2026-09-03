/********************************************************************************
*                                                                               *
*               P a r s e d   . F O N / . F N T   B i t m a p   F a c e         *
*                                                                               *
********************************************************************************/
#ifndef XFNTFACE_H
#define XFNTFACE_H

#ifndef FXARRAY_H
#include "FXArray.h"
#endif

// Prototype (Phase 3b): shared struct definitions for a parsed Windows
// .FON/.FNT bitmap font, used by both FXFont.cpp (parsing and metrics) and
// FXDCWindow.cpp (rendering). Internal to the library -- not installed with
// the public headers, not part of the public API. See PLAN.md, Phase 3.
namespace FX {

struct FXFntGlyph {
  FXushort width;    // pixel width (horizontal advance)
  FXuint   offset;   // byte offset of this glyph's bitmap, from the start of data[]
  };

struct FXFntFace {
  FXuchar    *data;         // raw bytes of the one selected FNT resource (owned)
  FXuint      datasize;     // size of data[], for bounds-checking glyph reads (untrusted file input)
  FXFntGlyph *glyphs;       // one entry per character in [firstChar,lastChar] (owned)
  FXint       firstChar;
  FXint       lastChar;
  FXint       defaultChar;
  FXint       pixHeight;    // glyph cell height
  FXint       ascent;
  FXint       maxWidth;
  FXint       avgWidth;
  };

// Prototype (Phase 4): one discoverable .FON/.FNT bitmap font "variant" --
// a single (family, weight, italic, size) combination found while scanning
// SETTINGS/bitmapfontpath, used by FXFontSelector to list bitmap fonts
// alongside Xft ones. `path` plus `points` (in deci-points, i.e. *10) is
// exactly what FXFont's create() needs to load it back
// (fntIsFonPath()/fntLoad() -- see FXFont.cpp) -- see PLAN.md, Phase 4.
struct FXBitmapFontEntry {
  FXString path;        // .fon file this came from
  FXString family;      // face name, as embedded in the FNT resource (dfFace)
  FXushort weight;      // FXFont::Normal or FXFont::Bold
  FXbool   italic;
  FXushort points;      // whole points (not deci-points)
  };

// Scan every "*.fon" file in each PATHLISTSEP-separated directory of
// searchpath, appending one FXBitmapFontEntry per embedded FNT resource
// found to out (which is *not* cleared first -- caller's choice whether to
// accumulate across several calls or out.clear() first). Returns true if
// at least one entry was found. Implemented in FXFont.cpp; a no-op stub
// outside the HAVE_XFT_H build (see PLAN.md, Phase 3b/4 -- .fon support
// only exists in the Xft-enabled build so far).
extern FXbool fxListBitmapFonts(const FXString& searchpath,FXArray<FXBitmapFontEntry>& out);

}

#endif
