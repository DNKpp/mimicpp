//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_PRINTING_TYPE_SIGNATURE_HPP
#define MIMICPP_PRINTING_TYPE_SIGNATURE_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/TypeTraits.hpp"
#include "mimic++/Utility.hpp"
#include "mimic++/printing/Format.hpp"
#include "mimic++/printing/Fwd.hpp"
#include "mimic++/printing/type/PrintType.hpp"

#include <functional>
#include <iterator>
#include <type_traits>

namespace mimicpp::printing::detail::type
{
    template <typename Out, typename... Ts>
    constexpr Out& print_separated(Out& out, const StringViewT separator, const type_list<Ts...> ts)
    {
        if constexpr (0u < sizeof...(Ts))
        {
            std::invoke(
                [&]<typename First, typename... Others>([[maybe_unused]] const type_list<First, Others...>) {
                    mimicpp::print_type<First>(std::ostreambuf_iterator{out});
                    ((out << separator, mimicpp::print_type<Others>(std::ostreambuf_iterator{out})), ...);
                },
                ts);
        }

        return out;
    }

    template <satisfies<std::is_function> Signature>
    struct signature_type_printer<Signature>
    {
        [[nodiscard]]
        static StringViewT name()
        {
            static const StringT name = generate_name();
            return name;
        }

    private:
        [[nodiscard]]
        static StringT generate_name()
        {
            StringStreamT out{};
            mimicpp::print_type<signature_return_type_t<Signature>>(std::ostreambuf_iterator{out});
            out << "(";
            print_separated(out, ", ", signature_param_list_t<Signature>{});
            out << ")";

            if constexpr (Constness::as_const == signature_const_qualification_v<Signature>)
            {
                out << " const";
            }

            if constexpr (ValueCategory::lvalue == signature_ref_qualification_v<Signature>)
            {
                out << " &";
            }
            else if constexpr (ValueCategory::rvalue == signature_ref_qualification_v<Signature>)
            {
                out << " &&";
            }

            if constexpr (signature_is_noexcept_v<Signature>)
            {
                out << " noexcept";
            }

            return std::move(out).str();
        }
    };
}

#endif
