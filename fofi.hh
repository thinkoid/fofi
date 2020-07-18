// -*- mode: c++; -*-
// Copyright 2009 Glyph & Cog, LLC
// Copyright 2019 Thinkoid, LLC

#ifndef FOFI_FOFI_HH
#define FOFI_FOFI_HH

#include <defs.hh>

namespace xpdf::fofi {

enum font_type {
    FONT_TYPE1_PFA,           // Type 1 font in PFA format
    FONT_TYPE1_PFB,           // Type 1 font in PFB format
    FONT_CFF_8BIT,            // 8-bit CFF font
    FONT_CFF_CID,             // CID CFF font
    FONT_TRUETYPE,            // TrueType font
    FONT_TRUETYPE_COLLECTION, // FONT_TRUETYPE collection
    FONT_OPENTYPE_CFF_8BIT,   // OpenType wrapper with 8-bit CFF font
    FONT_OPENTYPE_CFF_CID,    // OpenType wrapper with CID CFF font
    FONT_DFONT,               // Mac OSX dfont
    FONT_UNKNOWN,             // Unknown type
    FONT_ERROR
};

bool identify_byextension(const char *, xpdf::fofi::font_type &);
bool identify_bycontent(const char *, xpdf::fofi::font_type &);

bool identify(const char *, xpdf::fofi::font_type &);
bool identify(const char *, size_t, xpdf::fofi::font_type &);

} // namespace xpdf::fofi

#endif // FOFI_FOFI_HH
