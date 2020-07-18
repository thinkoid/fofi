// -*- mode: c++; -*-
// Copyright 2009 Glyph & Cog, LLC
// Copyright 2019 Thinkoid, LLC

#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

#include <boost/iostreams/device/mapped_file.hpp>
namespace io = boost::iostreams;

#include <fofi.hh>
#include <detail/fofi.hh>

namespace xpdf::fofi {

bool identify_byextension(const char *filepath, xpdf::fofi::font_type &result)
{
    if (fs::path(filepath).extension() == ".dfont")
        return result = xpdf::fofi::FONT_DFONT, true;

    return false;
}

bool identify_bycontent(const char *filepath, xpdf::fofi::font_type &result)
{
    if (fs::exists(filepath)) {
        io::mapped_file_source src(filepath);
        auto iter = src.begin(), last = src.end();
        return detail::identify(iter, last, result);
    }

    return result = FONT_ERROR, false;
}

bool identify(const char *filepath, xpdf::fofi::font_type &result)
{
    return identify_byextension(filepath, result) ||
           identify_bycontent(filepath, result);
}

bool identify(const char *pbuf, size_t n, xpdf::fofi::font_type &type)
{
    return detail::identify(pbuf, pbuf + n, type);
}

} // namespace xpdf::fofi

int main(int, char **argv)
{
    static const char *names[] = {
        "Type1 font in PFA format",
        "Type1 font in PFB format",
        "8-bit CFF font",
        "CID CFF font",
        "TrueType font",
        "TrueType font collection",
        "OpenType container of 8-bit CFF fonts",
        "OpenType container of CID-keyed CFF fonts",
        "(unknown)"
    };

    xpdf::fofi::font_type type;

    if (xpdf::fofi::identify(argv[1], type)) {
        std::cout << names[type] << std::endl;
        return 0;
    }

    std::cerr << "error" << std::endl;
    return 1;
}
