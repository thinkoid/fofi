// -*- mode: c++; -*-
// Copyright 2009 Glyph & Cog, LLC
// Copyright 2019 Thinkoid, LLC

#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

#include <boost/iostreams/device/mapped_file.hpp>
namespace io = boost::iostreams;

#include <fofi.hh>

static inline const char *name_of(xpdf::fofi::font_type arg)
{
    static const char *arr[] = {
        "Type1 font in PFA format",
        "Type1 font in PFB format",
        "8-bit CFF font",
        "CID CFF font",
        "TrueType font",
        "TrueType collection",
        "OpenType container of 8-bit CFF font",
        "OpenType container of CID-keyed CFF font",
        "(unknown)"
    };

    return arr[arg];
}

////////////////////////////////////////////////////////////////////////

static bool
identify_byextension(const char *filepath, xpdf::fofi::font_type &result)
{
    if (fs::path(filepath).extension() == ".dfont")
        return result = xpdf::fofi::DFont, true;

    return false;
}

static bool
identify_bycontent(const char *filepath, xpdf::fofi::font_type &result)
{
    io::mapped_file_source src(filepath);
    auto iter = src.begin(), last = src.end();
    return xpdf::fofi::identify(iter, last, result);
}

static bool
identify(const char *filepath, xpdf::fofi::font_type &result)
{
    return identify_byextension(filepath, result) ||
           identify_bycontent(filepath, result);
}

////////////////////////////////////////////////////////////////////////

int main(int, char **argv)
{
    xpdf::fofi::font_type result;

    if (identify(argv[1], result)) {
        std::cout << name_of(result) << std::endl;
        return 0;
    } else {
        std::cerr << "error" << std::endl;
        return true;
    }
}
