//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_PRINTING_FWD_HPP
#define MIMICPP_PRINTING_FWD_HPP

#pragma once

#include "mimic++/config/Config.hpp"

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::custom
{
    /**
     * \brief User may add specializations, which will then be used during ``print`` calls.
     * \ingroup PRINTING_STATE
     */
    template <typename>
    class Printer;

    /**
     * \brief User may add specializations that will be utilized during ``type_print`` calls.
     * \ingroup PRINTING_TYPE
     */
    template <typename>
    class TypePrinter;
}

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::printing
{
    class PrintFn;

    template <typename T>
    class PrintTypeFn;
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

namespace mimicpp::printing::type::detail
{
    template <typename T>
    struct common_type_printer;

    template <typename T>
    struct signature_type_printer;

    template <typename T>
    struct template_type_printer;
}

#endif
