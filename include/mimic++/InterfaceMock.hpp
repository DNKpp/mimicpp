//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_INTERFACE_MOCK_HPP
#define MIMICPP_INTERFACE_MOCK_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/Mock.hpp"
#include "mimic++/config/Config.hpp"
#include "mimic++/macros/Common.hpp"
#include "mimic++/macros/InterfaceMocking.hpp"
#include "mimic++/printing/TypePrinter.hpp"
#include "mimic++/utilities/StaticString.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <cstddef>
    #include <iterator>
    #include <tuple>
    #include <type_traits>
    #include <utility>
#endif

namespace mimicpp
{
    /**
     * \defgroup MOCK_INTERFACES interfaces
     * \ingroup MOCK
     * \brief Contains utility to simplify interface mocking.
     * \details While this library tries avoiding macros when possible, sometimes we must not be too stubborn.
     * Making interface mocking more enjoyable is such a situation. While this can of course be done without macros,
     * this quickly becomes annoying, due to the necessary boilerplate code.
     * \snippet InterfaceMock.cpp interface mock manual
     * ``mimic++`` therefore introduces some macros, which helps to reduce the effort to a minimum.
     * With them, the boilerplate can be reduced to this macro invocation, which effectively does the same as before:
     * ```cpp
     * MOCK_METHOD(foo, void, ());
     * ```
     *
     * The good news is, that these macros are just a thin layer around the macro free core and can thus be easily avoided.
     * Nevertheless, ``mimic++`` still aims to become macro-less as possible. As soon as reflection becomes available, an
     * attempt will be made to solve this feature completely in c++ language (hopefully with c++26, but only time will tell).
     *
     * ## Multiple inheritance
     * Multiple inheritance is fully supported, without any special tricks.
     * \snippet InterfaceMock.cpp interface mock multiple inheritance
     * \details
     * ## Mocks and variadic templates
     * Due to the nature of the ``mimicpp::Mock`` design, they directly supports packs without any question.
     * \snippet VariadicMocks.cpp variadic mock def
     * \snippet VariadicMocks.cpp variadic mock
     *
     * The interesting part is: Do ``MIMICPP_MOCK_METHOD`` and ``MIMICPP_MOCK_OVERLOADED_METHOD`` also support variadic templates?
     *
     * Yes they do! Both handle packs correctly.
     * \snippet VariadicMocks.cpp variadic interface def
     *
     * They can then be used with arbitrary template arguments.
     * \snippet VariadicMocks.cpp variadic interface zero
     * \snippet VariadicMocks.cpp variadic interface 2
     */
}

#endif
