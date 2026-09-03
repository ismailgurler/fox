/********************************************************************************
*                                                                               *
*               P a r s e d   . F O N / . F N T   B i t m a p   F a c e         *
*                                                                               *
********************************************************************************/
#ifndef XFNTFACE_H
#define XFNTFACE_H

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

// Note: FXBitmapFontEntry / FXFont::listBitmapFonts() / FXFont::isBitmapFontPath()
// used to live here too (Phase 4 prototype); promoted to the public
// FXFont.h/FXFont.cpp once bitmap fonts got wired into the shared
// FXFontSelector widget -- see PLAN.md.

}

#endif
