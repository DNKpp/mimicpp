//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_PRINTING_TYPE_SIGNATURE_HPP
#define MIMICPP_PRINTING_TYPE_SIGNATURE_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/TypeTraits.hpp"
#include "mimic++/config/Config.hpp"
#include "mimic++/printing/Format.hpp"
#include "mimic++/printing/Fwd.hpp"
#include "mimic++/printing/type/PrintType.hpp"
#include "mimic++/utilities/Concepts.hpp"
#include "mimic++/utilities/TypeList.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <algorithm>
    #include <functional>
    #include <iterator>
    #include <type_traits>
#endif

namespace mimicpp::printing::type::detail
{
    template <print_iterator OutIter, typename... Ts>
    constexpr OutIter print_separated(OutIter out, StringViewT const separator, util::type_list<Ts...> const ts)
    {
        if constexpr (0u < sizeof...(Ts))
        {
            std::invoke(
                [&]<typename First, typename... Others>([[maybe_unused]] util::type_list<First, Others...> const) {
                    out = mimicpp::print_type<First>(std::move(out));
                    ((out = mimicpp::print_type<Others>(std::ranges::copy(separator, std::move(out)).out)), ...);
                },
                ts);
        }

        return out;
    }

    template <util::satisfies<std::is_function> Signature>
    struct signature_type_printer<Signature>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out)
        {
            out = mimicpp::print_type<signature_return_type_t<Signature>>(std::move(out));
            out = format::format_to(std::move(out), "(");
            out = print_separated(out, ", ", signature_param_list_t<Signature>{});
            out = format::format_to(std::move(out), ")");

            if constexpr (Constness::as_const == signature_const_qualification_v<Signature>)
            {
                out = format::format_to(std::move(out), " const");
            }

            if constexpr (ValueCategory::lvalue == signature_ref_qualification_v<Signature>)
            {
                out = format::format_to(std::move(out), " &");
            }
            else if constexpr (ValueCategory::rvalue == signature_ref_qualification_v<Signature>)
            {
                out = format::format_to(std::move(out), " &&");
            }

            if constexpr (signature_is_noexcept_v<Signature>)
            {
                out = format::format_to(std::move(out), " noexcept");
            }

            return out;
        }
    };
}

#endif
