// -*- mode: c++ -*-
// Copyright 2019-2020 Thinkoid, LLC.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE xpdf

#include <defs.hh>

#include <iostream>
#include <exception>

#include <boost/type_index.hpp>
#include <boost/test/unit_test.hpp>
namespace utf = boost::unit_test;

#include <boost/test/data/test_case.hpp>
#include <boost/test/data/monomorphic.hpp>
namespace data = boost::unit_test::data;

# include <boost/iterator/iterator_adaptor.hpp>

#include <fofi.hh>
#include <detail/fofi.hh>

template< typename T >
struct forward_iterator : boost::iterator_adaptor<
    forward_iterator< T >, T*, boost::use_default,
    boost::forward_traversal_tag >
{
private:
    struct enabler{ };
    using base_type = typename forward_iterator::iterator_adaptor_;

public:
    forward_iterator() : base_type(0) { }

    explicit forward_iterator(T* p) : base_type(p)
    { }

    template< typename U >
    forward_iterator(
        const forward_iterator< U > &other,
        typename boost::enable_if<
            boost::is_convertible< U*, T* >, enabler >::type = enabler())
        : base_type(other.base())
    { }

private:
    friend class boost::iterator_core_access;

private:
    void increment()
    {
        this->base_reference() = this->base() + 1;
    }
};

BOOST_AUTO_TEST_SUITE(box)

static const char buf[] = "Hello, world!";
static const char *pbuf = buf, *pendbuf = buf + sizeof buf - 1;

static const std::vector<
    std::tuple< const char *, const char *, const char *, const char *,
                int, bool > >
forward_advance_dataset = {
    { pbuf,    pbuf, pendbuf, pbuf + 1, 1,  true },
    { pbuf, pendbuf, pendbuf, pendbuf,  1, false },
};

BOOST_DATA_TEST_CASE(
    forward_advance_,
    data::make(forward_advance_dataset), pbeg, p, pend, p2, off, result)
{
    using namespace xpdf::fofi::detail;

    using iterator = forward_iterator< char const >;
    iterator first(pbeg), iter(p), start(iter), last(pend);

    const auto b = safe_advance(first, iter, last, off);
    BOOST_CHECK(b == result);

    if (result) {
        BOOST_CHECK(off == std::distance(start, iter));
    }

    BOOST_CHECK(iter == iterator(p2));
}

template< typename T >
struct bidirectional_iterator : boost::iterator_adaptor<
    bidirectional_iterator< T >, T*, boost::use_default,
    boost::bidirectional_traversal_tag >
{
private:
    struct enabler{ };
    using base_type = typename bidirectional_iterator::iterator_adaptor_;

public:
    bidirectional_iterator() : base_type(0) { }

    explicit bidirectional_iterator(T* p) : base_type(p)
    { }

    template< typename U >
    bidirectional_iterator(
        const bidirectional_iterator< U > &other,
        typename boost::enable_if<
            boost::is_convertible< U*, T* >, enabler >::type = enabler())
        : base_type(other.base())
    { }

private:
    friend class boost::iterator_core_access;

private:
    void increment()
    {
        this->base_reference() = this->base() + 1;
    }

    void decrement()
    {
        this->base_reference() = this->base() - 1;
    }
};

static const std::vector<
    std::tuple< const char *, const char *, const char *, const char *,
                int, bool > >
bidirectional_advance_dataset = {
    { pbuf,    pbuf, pendbuf, pbuf + 1,     1,  true },
    { pbuf,    pbuf, pendbuf, pbuf,        -1, false },
    { pbuf, pendbuf, pendbuf, pendbuf,      1, false },
    { pbuf, pendbuf, pendbuf, pendbuf - 1, -1,  true },
};

BOOST_DATA_TEST_CASE(
    bidirectional_advance_,
    data::make(bidirectional_advance_dataset), pbeg, p, pend, p2, off, result)
{
    using namespace xpdf::fofi::detail;

    using iterator = bidirectional_iterator< char const >;
    iterator first(pbeg), iter(p), start(iter), last(pend);

    const auto b = safe_advance(first, iter, last, off);
    BOOST_CHECK(b == result);

    if (result) {
        if (off < 0) {
            BOOST_CHECK(off == -std::distance(iter, start));
        } else {
            BOOST_CHECK(off == std::distance(start, iter));
        }
    }

    BOOST_CHECK(iter == iterator(p2));
}

template< typename T >
struct random_access_iterator : boost::iterator_adaptor<
    random_access_iterator< T >, T*, boost::use_default,
    boost::random_access_traversal_tag >
{
private:
    struct enabler{ };
    using base_type = typename random_access_iterator::iterator_adaptor_;

public:
    random_access_iterator() : base_type(0) { }

    explicit random_access_iterator(T* p) : base_type(p)
    { }

    template< typename U >
    random_access_iterator(
        const random_access_iterator< U > &other,
        typename boost::enable_if<
            boost::is_convertible< U*, T* >, enabler >::type = enabler())
        : base_type(other.base())
    { }

private:
    friend class boost::iterator_core_access;

private:
    void increment()
    {
        this->base_reference() = this->base() + 1;
    }

    void decrement()
    {
        this->base_reference() = this->base() - 1;
    }

    void advance(typename random_access_iterator::difference_type n) {
        this->base_reference() = this->base() + n;
    }
};

static const std::vector<
    std::tuple< const char *, const char *, const char *, const char *,
                int, bool > >
random_access_advance_dataset = {
    { pbuf,    pbuf, pendbuf,  pbuf + 1,                       1,  true },
    { pbuf,    pbuf, pendbuf,  pbuf,                          -1, false },
    { pbuf, pendbuf, pendbuf,  pendbuf,                        1, false },
    { pbuf, pendbuf, pendbuf,  pendbuf - 1,                   -1,  true },
    { pbuf,    pbuf, pendbuf,  pendbuf,     (pendbuf - pbuf    ),  true },
    { pbuf,    pbuf, pendbuf,  pendbuf,     (pendbuf - pbuf + 1), false },
    { pbuf, pendbuf, pendbuf,  pbuf,       -(pendbuf - pbuf    ),  true },
    { pbuf, pendbuf, pendbuf,  pbuf,       -(pendbuf - pbuf + 1), false },
};

BOOST_DATA_TEST_CASE(
    random_access_advance_,
    data::make(random_access_advance_dataset), pbeg, p, pend, p2, off, result)
{
    using namespace xpdf::fofi::detail;

    using iterator = random_access_iterator< char const >;
    iterator first(pbeg), iter(p), start(iter), last(pend);

    const auto b = safe_advance(first, iter, last, off);
    BOOST_CHECK(b == result);

    if (result) {
        if (off < 0) {
            BOOST_CHECK(off == -std::distance(iter, start));
        } else {
            BOOST_CHECK(off == std::distance(start, iter));
        }
    }

    BOOST_CHECK(iter == iterator(p2));
}

BOOST_AUTO_TEST_SUITE_END()
