// -*- mode: c++; -*-
// Copyright 2009 Glyph & Cog, LLC
// Copyright 2019 Thinkoid, LLC

#ifndef FOFI_FOFI_HH
#define FOFI_FOFI_HH

#include <defs.hh>

#include <iterator>

#include <boost/endian/arithmetic.hpp>
#include <boost/endian/conversion.hpp>

namespace xpdf {
namespace fofi {

enum font_type {
    Type1PFA, // Type 1 font in PFA format
    Type1PFB, // Type 1 font in PFB format
    CFF8Bit, // 8-bit CFF font
    CFFCID, // CID CFF font
    TrueType, // TrueType font
    TrueTypeCollection, // TrueType collection
    OpenTypeCFF8Bit, // OpenType wrapper with 8-bit CFF font
    OpenTypeCFFCID, // OpenType wrapper with CID CFF font
    DFont, // Mac OS X dfont
    UnknownType // unknown type
};

} // namespace fofi

namespace parser {
namespace detail {

template< typename Iterator >
struct iterator_guard_t
{
    iterator_guard_t(Iterator &iter)
        : iter(iter), save(iter), restore(true)
    { }

    ~iterator_guard_t()
    {
        if (restore)
            iter = save;
    }

    void release() { restore = false; }

    Iterator &iter, save;
    bool restore;
};

#define ITERATOR_GUARD(x) iterator_guard_t iterator_guard(x)
#define ITERATOR_RELEASE iterator_guard.release()
#define PARSE_SUCCESS                           \
    ITERATOR_RELEASE;                           \
    return true

#define S_(x) std::string(x, sizeof x - 1)

#define ITERATOR_CONDITIONAL(x)                                                  \
    template< typename T >                                                       \
    constexpr auto is_##x##_iterator =                                           \
        std::is_same_v< typename std::iterator_traits< T >::iterator_category,   \
                        std::x##_iterator_tag >

ITERATOR_CONDITIONAL(input);
ITERATOR_CONDITIONAL(output);
ITERATOR_CONDITIONAL(forward);
ITERATOR_CONDITIONAL(bidirectional);
ITERATOR_CONDITIONAL(random_access);

#undef ITERATOR_CONDITIONAL

template< typename Iterator, typename Distance >
inline typename std::enable_if_t< is_forward_iterator< Iterator >, bool >
safe_advance(Iterator first, Iterator &iter, Iterator last, Distance n)
{
    ASSERT(0 == n || 0 < n);
    for (; n && iter != last; --n, ++iter) ;
    return 0 == n;
}

template< typename Iterator, typename Distance >
inline typename std::enable_if_t< is_bidirectional_iterator< Iterator >, bool >
safe_advance(Iterator first, Iterator &iter, Iterator last, Distance n)
{
    if (n > 0)
        for (; n && iter != last; --n, ++iter) ;
    else if (n < 0)
        for (; n && iter != last; ++n, --iter) ;

    return 0 == n;
}

template< typename Iterator, typename Distance >
inline typename std::enable_if_t< is_random_access_iterator< Iterator >, bool >
safe_advance(Iterator first, Iterator &iter, Iterator last, Distance n)
{
    using difference_type = typename std::iterator_traits<
        Iterator >::difference_type;

    const auto off = n < 0
        ? std::max(difference_type(n), std::distance(iter, first))
        : std::min(difference_type(n), std::distance(iter, last));

    iter += off;
    n -= off;

    return 0 == n;
}

template< typename T > void big_to_native_inplace(T &arg, size_t size)
{
    namespace endian = boost::endian;

    if (size == sizeof arg)
        endian::big_to_native_inplace(arg);
    else {
        T     dst = 0;
        char *pdst = reinterpret_cast< char * >(&dst) + size - 1;

        const char *psrc = reinterpret_cast< char * >(&arg);
        const char *pend = psrc + size;

        for (; psrc != pend; ++psrc, --pdst)
            pdst[0] = psrc[0];

        arg = dst;
    }
}

} // namespace detail

template< typename Iterator >
bool literal_char(Iterator &iter, Iterator last, char c)
{
    using namespace detail;
    ITERATOR_GUARD(iter);

    if (iter != last) {
        auto x = *iter++;

        if (x == c) {
            PARSE_SUCCESS;
        }
    }

    return false;
}

template< typename Iterator >
bool literal_string(Iterator &iter, Iterator last, const std::string &s)
{
    using namespace detail;
    ITERATOR_GUARD(iter);

    auto other = s.begin();
    for (; iter != last && other != s.end() && *iter == *other; ++iter, ++other)
        ;

    if (other == s.end()) {
        PARSE_SUCCESS;
    }

    return false;
}

template< typename Iterator >
bool literal_int(Iterator &iter, Iterator last, int &i)
{
    using namespace detail;
    ITERATOR_GUARD(iter);

    if (iter != last) {
        int x = 0;

        if (!digit(iter, last, x))
            return false;

        int val = x;

        for (; iter != last;) {
            if (!digit(iter, last, x))
                break;

            val *= 10;
            val += x;
        }

        i = val;

        PARSE_SUCCESS;
    }

    return false;
}

template< typename Iterator, typename T >
bool sized_integral(Iterator &iter, Iterator last, T &attr, size_t n)
{
    ASSERT(n <= sizeof attr);

    using namespace detail;
    ITERATOR_GUARD(iter);

    if (iter != last) {
        size_t i = 0, value = 0;

        for (; i < n && iter != last; ++i, ++iter)
            reinterpret_cast< char * >(&value)[i] = *iter;

        if (i < n)
            return false;

        attr = value;

        PARSE_SUCCESS;
    }

    return false;
}

template< typename Iterator, typename T >
bool integral(Iterator &iter, Iterator last, T &attr)
{
    return sized_integral(iter, last, attr, sizeof attr);
}

} // namespace parser

////////////////////////////////////////////////////////////////////////

namespace fofi {

template< typename Iterator >
bool identify_pfa(Iterator &iter, Iterator last, font_type &result)
{
    using namespace xpdf::parser;
    
    if (literal_string(iter, last, "%!PS-AdobeFont-1") ||
        literal_string(iter, last, "%!FontType1")) {
        result = Type1PFA;
        return true;
    }

    return false;
}

template< typename Iterator >
bool identify_pfb(Iterator &iter, Iterator last, font_type &result)
{
    using namespace xpdf::parser;
    using namespace xpdf::parser::detail;

    ITERATOR_GUARD(iter);

    if (literal_string(iter, last, "\x80\x01")) {
        unsigned n = 0;

        if (integral(iter, last, n)) {
            if ((n >= 16 && literal_string(iter, last, "%!PS-AdobeFont-1")) ||
                (n >= 11 && literal_string(iter, last, "%!FontType1"))) {
                result = Type1PFB;
                PARSE_SUCCESS;
            }
        }
    }

    return false;
}

template< typename Iterator >
bool identify_ttf(Iterator &iter, Iterator last, font_type &result)
{
    using namespace xpdf::parser;
    
    if (literal_string(iter, last, std::string("\x00\x01\x00\x00", 4)) ||
        literal_string(iter, last, "true")) {
        result = TrueType;
        return true;
    }

    if (literal_string(iter, last, "ttcf")) {
        result = TrueTypeCollection;
        return true;
    }

    return false;
}

template< typename Iterator >
bool identify_cff(Iterator &iter, Iterator last, font_type &result)
{
    namespace endian = boost::endian;
   
    using namespace xpdf::parser;
    using namespace xpdf::parser::detail;
    
    ITERATOR_GUARD(iter);
    const auto first = iter;

    if (!literal_string(iter, last, std::string("\x01\x00", 2)))
        return false;

    {
        unsigned char a = 0, b = 0;

        if (!integral(iter, last, a) || !integral(iter, last, b))
            return false;

        if (b < 1 || 4 < b || a < 4)
            return false;

        if (!safe_advance(first, iter, last, int(a) - 4))
            return false;
    }

    {
        unsigned short n;

        if (!integral(iter, last, n))
            return false;

        endian::big_to_native_inplace(n);

        if (n) {
            unsigned char x;

            if (!integral(iter, last, x))
                return false;

            if (!safe_advance(first, iter, last, n * x))
                return false;

            unsigned long y = 0;

            if (!sized_integral(iter, last, y, x))
                return false;

            big_to_native_inplace(y, x);

            if (!safe_advance(first, iter, last, long(y) - 1))
                return false;
        }
    }

    {
        unsigned short n = 0;

        if (!integral(iter, last, n) || 0 == n)
            return false;

        endian::big_to_native_inplace(n);

        unsigned char x = 0;

        if (!integral(iter, last, x))
            return false;

        unsigned long y = 0, z = 0;

        if (!sized_integral(iter, last, y, x) ||
            !sized_integral(iter, last, z, x) || y > z)
            return false;

        big_to_native_inplace(y, x);
        big_to_native_inplace(z, x);

        {
            auto end = last;
            last = iter;

            if (!safe_advance(first, iter, end, (n - 1) * x + y - 1) ||
                !safe_advance(first, last, end, (n - 1) * x + z - 1))
                return false;
        }

        for (size_t i = 0; i < 3; ++i) {
            unsigned char c = 0;

            if (!integral(iter, last, c))
                return false;

            if (c == 0x1c) {
                if (!safe_advance(first, iter, last, 2))
                    return false;
            } else if (c == 0x1d) {
                if (!safe_advance(first, iter, last, 4))
                    return false;
            } else if (c >= 0xf7 && c <= 0xfe) {
                if (!safe_advance(first, iter, last, 1))
                    return false;
            } else if (c < 0x20 || c > 0xf6) {
                result = CFF8Bit;
                PARSE_SUCCESS;
            }

            if (iter == last) {
                result = CFF8Bit;
                PARSE_SUCCESS;
            }
        }

        unsigned char c = 0;

        if (integral(iter, last, c) && c == 12 && integral(iter, last, c) &&
            c == 30) {
            result = CFFCID;
        } else {
            result = CFF8Bit;
        }

        PARSE_SUCCESS;
    }

    return false;
}

template< typename Iterator >
bool identify_otf(Iterator &iter, Iterator last, font_type &result)
{
    namespace endian = boost::endian;

    using namespace xpdf::parser;
    using namespace xpdf::parser::detail;

    ITERATOR_GUARD(iter);   
    const auto first = iter;

    if (literal_string(iter, last, "OTTO")) {
        int short n = 0;

        if (integral(iter, last, n)) {
            endian::big_to_native_inplace(n);
            assert(n >= 0);

            if (!safe_advance(first, iter, last, 6))
                return false;

            for (size_t i = 0; i < size_t(n); ++i) {
                if (literal_string(iter, last, "CFF ")) {
                    if (!safe_advance(first, iter, last, 4))
                        break;

                    int off = 0;

                    if (integral(iter, last, off)) {
                        endian::big_to_native_inplace(off);

                        if (off < INT_MAX) {
                            auto iter2 = first;

                            if (!safe_advance(first, iter2, last, off))
                                break;

                            if (identify_cff(iter2, last, result)) {
                                switch (result) {
                                case CFF8Bit:
                                    result = OpenTypeCFF8Bit;
                                    PARSE_SUCCESS;

                                case CFFCID:
                                    result = OpenTypeCFFCID;
                                    PARSE_SUCCESS;

                                default:
                                    break;
                                }
                            }
                        }
                    }

                    return false;
                }

                if (!safe_advance(first, iter, last, 16))
                    break;
            }
        }
    }

    return false;
}

template< typename Iterator >
bool identify(Iterator &iter, Iterator last, font_type &result)
{
    return
        identify_pfa(iter, last, result) ||
        identify_pfb(iter, last, result) ||
        identify_ttf(iter, last, result) ||
        identify_otf(iter, last, result) ||
        identify_cff(iter, last, result);
}

} // namespace fofi
} // namespace xpdf

#endif // FOFI_FOFI_HH
