//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_PRINTING_STATE_COMMON_TYPES_HPP
#define MIMICPP_PRINTING_STATE_COMMON_TYPES_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/String.hpp"
#include "mimic++/TypeTraits.hpp" // uint_with_size
#include "mimic++/printing/Format.hpp"
#include "mimic++/printing/Fwd.hpp"
#include "mimic++/printing/PathPrinter.hpp"
#include "mimic++/printing/state/Print.hpp"

#include <algorithm>
#include <bit>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <memory>
#include <optional>
#include <source_location>
#include <type_traits>
#include <utility>

namespace mimicpp::printing::detail::state
{
    template <>
    struct common_type_printer<std::source_location>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out, std::source_location const& loc)
        {
            out = format::format_to(std::move(out), "`");
            out = print_path(std::move(out), loc.file_name());
            out = format::format_to(std::move(out), "`");

            return format::format_to(
                std::move(out),
                "#L{}, `{}`",
                loc.line(),
                loc.function_name());
        }
    };

    template <>
    struct common_type_printer<std::nullopt_t>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out, [[maybe_unused]] std::nullopt_t const)
        {
            return format::format_to(std::move(out), "nullopt");
        }
    };

    template <typename T>
    struct common_type_printer<std::optional<T>>
    {
        template <print_iterator OutIter>
        static constexpr OutIter print(OutIter out, auto& opt)
        {
            if (opt)
            {
                return mimicpp::print(std::move(out), *opt);
            }

            return mimicpp::print(std::move(out), std::nullopt);
        }
    };

    template <>
    struct common_type_printer<std::nullptr_t>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out, [[maybe_unused]] std::nullptr_t const& ptr)
        {
            return format::format_to(std::move(out), "nullptr");
        }
    };

    template <typename T>
    struct common_type_printer<T*>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out, T const* ptr)
        {
            if (auto const* inner = std::to_address(ptr))
            {
                auto const value = std::bit_cast<std::uintptr_t>(inner);
                if constexpr (4u < sizeof(std::uintptr_t))
                {
                    return format::format_to(
                        std::move(out),
                        "0x{:0>16x}",
                        value);
                }
                else
                {
                    return format::format_to(
                        std::move(out),
                        "0x{:0>8x}",
                        value);
                }
            }

            return mimicpp::print(std::move(out), nullptr);
        }
    };

    template <typename T>
    concept smart_pointer = requires(T const& ptr) {
        typename T::element_type;
        { ptr.get() } -> std::convertible_to<typename T::element_type*>;
    };

    template <smart_pointer T>
    struct common_type_printer<T>
    {
        template <print_iterator OutIter>
        static constexpr OutIter print(OutIter out, T const& ptr)
        {
            return mimicpp::print(std::move(out), ptr.get());
        }
    };

    template <typename T>
    struct common_type_printer<std::weak_ptr<T>>
    {
        template <print_iterator OutIter>
        static constexpr OutIter print(OutIter out, std::weak_ptr<T> const& ptr)
        {
            return mimicpp::print(std::move(out), ptr.lock());
        }
    };

    template <string String>
    struct common_type_printer<String>
    {
        template <std::common_reference_with<String> T, print_iterator OutIter>
        static OutIter print(OutIter out, T& str)
        {
            if constexpr (constexpr auto prefix = string_literal_prefix<string_char_t<String>>;
                          !std::ranges::empty(prefix))
            {
                out = std::ranges::copy(prefix, std::move(out)).out;
            }

            out = format::format_to(std::move(out), "\"");

            if constexpr (std::same_as<CharT, string_char_t<String>>)
            {
                out = std::ranges::copy(
                          string_traits<String>::view(str),
                          std::move(out))
                          .out;
            }
            // required for custom char types
            else if constexpr (printer_for<custom::Printer<string_char_t<String>>, OutIter, string_char_t<String>>)
            {
                for (custom::Printer<string_char_t<String>> printer{};
                     const string_char_t<String>& c : string_traits<String>::view(str))
                {
                    out = printer.print(std::move(out), c);
                }
            }
            else
            {
                constexpr auto to_dump = [](const string_char_t<String>& c) noexcept {
                    using intermediate_t = uint_with_size_t<sizeof c>;
                    return std::bit_cast<intermediate_t>(c);
                };

                auto view = string_traits<std::remove_cvref_t<T>>::view(std::forward<T>(str));
                auto iter = std::ranges::begin(view);
                if (const auto end = std::ranges::end(view);
                    iter != end)
                {
                    out = format::format_to(
                        std::move(out),
                        "{:#x}",
                        to_dump(*iter++));

                    for (; iter != end; ++iter)
                    {
                        out = format::format_to(
                            std::move(out),
                            ", {:#x}",
                            to_dump(*iter));
                    }
                }
            }

            return format::format_to(std::move(out), "\"");
        }
    };

    template <>
    struct common_type_printer<ValueCategory>
    {
        template <print_iterator OutIter>
        static constexpr OutIter print(OutIter out, ValueCategory const category)
        {
            constexpr auto toString = [](ValueCategory const cat) -> StringViewT {
                switch (cat)
                {
                case ValueCategory::lvalue: return {"lvalue"};
                case ValueCategory::rvalue: return {"rvalue"};
                case ValueCategory::any:    return {"any"};
                    // GCOVR_EXCL_START
                default:
                    unreachable();
                    // GCOVR_EXCL_STOP
                }
            };

            return std::ranges::copy(toString(category), std::move(out)).out;
        }
    };

    template <>
    struct common_type_printer<Constness>
    {
        template <print_iterator OutIter>
        static constexpr OutIter print(OutIter out, Constness const constness)
        {
            constexpr auto toString = [](Constness const value) -> StringViewT {
                switch (value)
                {
                case Constness::non_const: return {"mutable"};
                case Constness::as_const:  return {"const"};
                case Constness::any:       return {"any"};
                    // GCOVR_EXCL_START
                default:
                    unreachable();
                    // GCOVR_EXCL_STOP
                }
            };

            return std::ranges::copy(toString(constness), std::move(out)).out;
        }
    };
}

#endif
