//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_PRINTING_TYPE_PRINT_TYPE_HPP
#define MIMICPP_PRINTING_TYPE_PRINT_TYPE_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/printing/Format.hpp"
#include "mimic++/printing/Fwd.hpp"
#include "mimic++/printing/type/PostProcessing.hpp"
#include "mimic++/utilities/PriorityTag.hpp"

#include <algorithm>
#include <cassert>
#include <concepts>
#include <functional>
#include <iterator>
#include <type_traits>
#include <typeinfo>
#include <utility>

namespace mimicpp::printing::type
{
    /**
     * \brief Returns the (potentially demangled) name.
     * \ingroup PRINTING_TYPE
     * \tparam T The desired type.
     * \return The (potentially demangled) name.
     * \details This function serves as a wrapper for typeid(T).name().
     * - On MSVC, this function returns the demangled name directly.
     * - However, on GCC and Clang, the behavior differs.
     * When `MIMICPP_CONFIG_EXPERIMENTAL_PRETTY_TYPES` is enabled, it further demangles the name using `abi::__cxa_demangle`.
     */
    template <typename T>
    StringT type_name();
}

#if defined(MIMICPP_CONFIG_EXPERIMENTAL_PRETTY_TYPES) \
    && (MIMICPP_DETAIL_IS_GCC || MIMICPP_DETAIL_IS_CLANG)

    #include <cstdlib>
    #include <cxxabi.h>
    #include <memory>

namespace mimicpp::printing::type
{
    template <typename T>
    StringT type_name()
    {
        auto* const rawName = typeid(T).name();

        // see: https://gcc.gnu.org/onlinedocs/libstdc++/manual/ext_demangling.html
        int status{};
        using free_deleter_t = decltype([](char* c) noexcept { std::free(c); });
        std::unique_ptr<char, free_deleter_t> const demangledName{
            abi::__cxa_demangle(rawName, nullptr, nullptr, &status)};
        if (0 == status)
        {
            return {demangledName.get()};
        }

        return {rawName};
    }
}

#else

namespace mimicpp::printing::type
{
    template <typename T>
    StringT type_name()
    {
        return typeid(T).name();
    }
}

#endif

namespace mimicpp::printing::type::detail
{
    template <typename Printer, typename T, typename OutIter>
    concept type_printer_for = print_iterator<OutIter>
                            && requires(OutIter out) {
                                   typename Printer;
                                   { Printer::print(out) } -> std::convertible_to<OutIter>;
                               };

    template <typename T, print_iterator OutIter, type_printer_for<T, OutIter> Printer = mimicpp::custom::TypePrinter<T>>
    constexpr OutIter print_type_to([[maybe_unused]] util::priority_tag<4u> const, OutIter out)
    {
        return Printer::print(std::move(out));
    }

    template <typename T, print_iterator OutIter, type_printer_for<T, OutIter> Printer = common_type_printer<T>>
    constexpr OutIter print_type_to([[maybe_unused]] util::priority_tag<3u> const, OutIter out)
    {
        return Printer::print(std::move(out));
    }

    template <typename T, print_iterator OutIter, type_printer_for<T, OutIter> Printer = signature_type_printer<T>>
    constexpr OutIter print_type_to([[maybe_unused]] util::priority_tag<2u> const, OutIter out)
    {
        return Printer::print(std::move(out));
    }

    template <typename T, print_iterator OutIter, type_printer_for<T, OutIter> Printer = template_type_printer<T>>
    constexpr OutIter print_type_to([[maybe_unused]] util::priority_tag<1u> const, OutIter out)
    {
        return Printer::print(std::move(out));
    }

    template <typename T, print_iterator OutIter>
    OutIter print_type_to([[maybe_unused]] util::priority_tag<0u> const, OutIter out)
    {
        return type::prettify_identifier(
            std::move(out),
            type::type_name<T>());
    }

    constexpr util::priority_tag<4u> maxPrintTypePriority{};

    template <typename T>
    struct print_type_helper
    {
        template <print_iterator OutIter>
        [[nodiscard]]
        constexpr OutIter operator()(OutIter out) const
        {
            return print_type_to<T>(maxPrintTypePriority, std::move(out));
        }
    };

    template <typename T>
        requires(!std::is_array_v<T>)
    struct print_type_helper<volatile T>
    {
        template <print_iterator OutIter>
        [[nodiscard]]
        constexpr OutIter operator()(OutIter out) const
        {
            return format::format_to(
                print_type_helper<T>{}(std::move(out)),
                " volatile");
        }
    };

    template <typename T>
        requires(!std::is_array_v<T>)
    struct print_type_helper<const T>
    {
        template <print_iterator OutIter>
        [[nodiscard]]
        constexpr OutIter operator()(OutIter out) const
        {
            return format::format_to(
                print_type_helper<T>{}(std::move(out)),
                " const");
        }
    };

    template <typename T>
        requires(!std::is_array_v<T>)
    struct print_type_helper<const volatile T>
    {
        template <print_iterator OutIter>
        [[nodiscard]]
        constexpr OutIter operator()(OutIter out) const
        {
            return format::format_to(
                print_type_helper<T>{}(std::move(out)),
                " const volatile");
        }
    };

    template <typename T>
    struct print_type_helper<T&>
    {
        template <print_iterator OutIter>
        [[nodiscard]]
        constexpr OutIter operator()(OutIter out) const
        {
            return format::format_to(
                print_type_helper<T>{}(std::move(out)),
                "&");
        }
    };

    template <typename T>
    struct print_type_helper<T&&>
    {
        template <print_iterator OutIter>
        [[nodiscard]]
        constexpr OutIter operator()(OutIter out) const
        {
            return format::format_to(
                print_type_helper<T>{}(std::move(out)),
                "&&");
        }
    };

    template <typename T>
    struct print_type_helper<T*>
    {
        template <print_iterator OutIter>
        [[nodiscard]]
        constexpr OutIter operator()(OutIter out) const
        {
            return format::format_to(
                print_type_helper<T>{}(std::move(out)),
                "*");
        }
    };

    template <typename T>
    struct print_type_helper<T[]>
    {
        template <print_iterator OutIter>
        [[nodiscard]]
        constexpr OutIter operator()(OutIter out) const
        {
            return format::format_to(
                print_type_helper<T>{}(std::move(out)),
                "[]");
        }
    };

    template <typename T, std::size_t n>
    struct print_type_helper<T[n]>
    {
        template <print_iterator OutIter>
        [[nodiscard]]
        constexpr OutIter operator()(OutIter out) const
        {
            return format::format_to(
                print_type_helper<T>{}(std::move(out)),
                "[{}]",
                n);
        }
    };

    // multidimensional arrays are evaluated from left to right,
    // but we need it from right to left.
    template <typename T, std::size_t n, std::size_t m>
    struct print_type_helper<T[n][m]>
    {
        template <print_iterator OutIter>
        [[nodiscard]]
        constexpr OutIter operator()(OutIter out) const
        {
            return format::format_to(
                print_type_helper<T[n]>{}(std::move(out)),
                "[{}]",
                m);
        }
    };
}

namespace mimicpp::printing
{
    template <typename T>
    class PrintTypeFn
    {
    public:
        template <print_iterator OutIter>
        constexpr OutIter operator()(OutIter out) const
        {
            return type::detail::print_type_helper<T>{}(std::move(out));
        }

        [[nodiscard]]
        StringT operator()() const
        {
            StringStreamT stream{};
            std::invoke(*this, std::ostreambuf_iterator{stream});
            return std::move(stream).str();
        }
    };
}

namespace mimicpp
{
    /**
     * \defgroup PRINTING_TYPE object-type stringification
     * \ingroup PRINTING
     * \brief State stringification occurs when an object's type is transformed into a textual representation.
     * \details Type stringification requires special care in C++,
     * as there is no portable way to print types exactly as users would write them in code.
     * To address this, ``mimic++`` introduces the ``print_type<T>`` function,
     * which internally removes much of the extraneous noise that arises from various standard implementations.
     *
     * Furthermore, ``mimic++`` aims to present the majority of types in a manner that closely resembles how users would write them.
     * For class types, this is achievable with some additional text manipulation;
     * however, there is no way to determine whether users used the actual class name or an alias.
     * For example, ``std::string`` is actually the type ``std::basic_string<char, std::char_traits<char>, std::allocator<char>>``,
     * which introduces a significant amount of noise.
     *
     * Since it is common for users to write ``std::string`` rather than using the ``std::basic_string`` template form,
     * ``mimic++`` specifically handles these common types and will always print ``std::string`` when the exact long form is detected.
     *
     * However, there are dozens of template types in the STL and even more in user code; and mimic++ strives to handle them correctly.
     * When the name of a template type is requested, ``mimic++`` examines each of its arguments recursively and
     * even folds all default arguments.
     *
     * For example, the type
     * ```cpp
     * std::map<
     *     int,
     *     std::vector<float, std::allocator<float>>,
     *     std::less<int>,
     *     std::allocator<std::pair<int const, std::vector<float, std::allocator<float>>>>>
     * ```
     * will be printed as ``std::map<int, std::vector<float>>`` which is likely how users have written it.
     *
     * ### Customize type-stringification
     *
     * ``mimic++`` automatically converts every type into a meaningful textual representation,
     * but users may sometimes want to customize this process.
     * Users can create a specialization of  ``mimicpp::custom::TypePrinter`` for their any type, which will then be
     * prioritized over the internal conversions.
     *
     * Consider the following type.
     * \snippet CustomPrinter.cpp my_type
     * Users can then create a specialization as follows:
     * \snippet CustomPrinter.cpp my_type type-printer
     *
     * When the name of the (potentially cv-ref qualified) ``my_type`` is requested via ``print_type``, that specification will be used:
     * \snippet CustomPrinter.cpp my_type type-print
     *
     *\{
     */

    /**
     * \brief Functional object, converting the given type to its textual representation.
     */
    template <typename T>
    [[maybe_unused]] inline constexpr printing::PrintTypeFn<T> print_type{};

    /**
     * \}
     */
}

#endif
