//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_PRINTING_FWD_HPP
#define MIMICPP_PRINTING_FWD_HPP

namespace mimicpp::custom
{
    /**
     * \brief User may add specializations, which will then be used during ``print`` calls.
     * \ingroup PRINTING_STATE
     */
    template <typename>
    class Printer;
}

namespace mimicpp::printing::detail::state
{
    template <typename T>
    struct common_type_printer;

    template <typename T>
    struct cxx23_backport_printer;

    template <typename T>
    struct formattable_type_printer;

    template <typename T>
    struct unknown_type_printer;
}

namespace mimicpp::printing
{
    class PrintFn;
}

#endif
