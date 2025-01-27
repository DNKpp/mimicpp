//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_PRINTING_TYPE_PRINT_TYPE_HPP
#define MIMICPP_PRINTING_TYPE_PRINT_TYPE_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/Utility.hpp"
#include "mimic++/printing/Format.hpp"
#include "mimic++/printing/Fwd.hpp"

#include <algorithm>
#include <concepts>
#include <functional>
#include <iterator>
#include <type_traits>
#include <typeinfo>
#include <utility>

#ifndef MIMICPP_CONFIG_MINIMAL_PRETTY_TYPE_PRINTING

    #include <regex>

namespace mimicpp
{
    using RegexT = std::regex;
}

namespace mimicpp::printing::detail
{
    inline StringT regex_replace_all(StringT str, const RegexT& regex, const StringViewT fmt)
    {
        using match_result_t = std::match_results<StringT::const_iterator>;

        auto processedIter = str.cbegin();
        while (processedIter != str.cend())
        {
            if (match_result_t match{};
                std::regex_search(processedIter, str.cend(), match, regex))
            {
                const auto matchBegin = match[0].first;
                const auto matchEnd = match[0].second;
                str.replace(matchBegin, matchEnd, fmt);

                // just advance by one, so that we can find matches which involves the replaced fmt
                // like: `> > >` with regex `> >` and fmt = `>>`
                //      => `>> >` => `>>>`
                processedIter = std::ranges::next(matchBegin);
            }
            else
            {
                processedIter = str.cend();
            }
        }

        return str;
    }
}

    #if MIMICPP_DETAIL_IS_GCC \
        || MIMICPP_DETAIL_IS_CLANG

        #include <cstdlib>
        #include <cxxabi.h>
        #include <memory>

namespace mimicpp::printing::detail
{
    template <typename T>
    StringT type_name()
    {
        StringT name{typeid(T).name()};

        // see: https://gcc.gnu.org/onlinedocs/libstdc++/manual/ext_demangling.html
        int status{};
        using free_deleter_t = decltype([](char* c) noexcept { std::free(c); });
        std::unique_ptr<char, free_deleter_t> const demangledName{
            abi::__cxa_demangle(name.data(), nullptr, nullptr, &status)};
        if (0 == status)
        {
            name.assign(demangledName.get());
        }

        return name;
    }

    inline StringT prettify_type_name(StringT name)
    {
        static const RegexT unifyClosingAngleBrackets{R"(\>\s*\>)"};
        name = regex_replace_all(name, unifyClosingAngleBrackets, ">>");

        #if MIMICPP_DETAIL_USES_LIBCXX
        static const RegexT handleStdNamespace{"std::__1::"};
        #else
        static const RegexT handleStdNamespace{"std::__cxx11::"};
        #endif
        name = std::regex_replace(name, handleStdNamespace, "std::");

        static const RegexT omitAnonymousNamespace{R"(\(anonymous namespace\)::)"};
        name = std::regex_replace(name, omitAnonymousNamespace, "");

        return name;
    }
}

    #else

namespace mimicpp::printing::detail
{
    template <typename T>
    StringT type_name()
    {
        return typeid(T).name();
    }

    inline StringT prettify_type_name(StringT name)
    {
        static const RegexT unifyClosingAngleBrackets{R"(\>\s*\>)"};
        name = regex_replace_all(name, unifyClosingAngleBrackets, ">>");

        static const RegexT unifyComma{R"(\s*,\s*)"};
        name = std::regex_replace(name, unifyComma, ", ");

        static const RegexT omitClassStructEnum{R"(\b(class|struct|enum)\s+)"};
        name = std::regex_replace(name, omitClassStructEnum, "");

        static const RegexT omitAnonymousNamespace{"`anonymous namespace'::"};
        name = std::regex_replace(name, omitAnonymousNamespace, "");

        return name;
    }
}

    #endif

#else

namespace mimicpp::printing::detail
{
    template <typename T>
    StringT type_name()
    {
        return typeid(T).name();
    }

    [[nodiscard]]
    inline StringT prettify_type_name(StringT name) noexcept
    {
        return name;
    }
}

#endif

namespace mimicpp::printing::detail::type
{
    template <typename Printer, typename T, typename OutIter>
    concept type_printer_for = print_iterator<OutIter>
                            && (requires {
        typename Printer;
        { Printer::name() } -> std::convertible_to<StringViewT>; } || requires(OutIter out) {
        typename Printer;
        { Printer::print(out) } -> std::convertible_to<OutIter>; });

    template <typename T, print_iterator OutIter, type_printer_for<T, OutIter> Printer = mimicpp::custom::TypePrinter<T>>
    constexpr OutIter print_type_to([[maybe_unused]] const priority_tag<4u>, OutIter out)
    {
        return std::ranges::copy(Printer::name(), std::move(out))
            .out;
    }

    template <typename T, print_iterator OutIter, type_printer_for<T, OutIter> Printer = common_type_printer<T>>
    constexpr OutIter print_type_to([[maybe_unused]] const priority_tag<3u>, OutIter out)
    {
        return Printer::print(std::move(out));
    }

    template <typename T, print_iterator OutIter, type_printer_for<T, OutIter> Printer = signature_type_printer<T>>
    constexpr OutIter print_type_to([[maybe_unused]] const priority_tag<2u>, OutIter out)
    {
        return std::ranges::copy(Printer::name(), std::move(out))
            .out;
    }

    template <typename T, print_iterator OutIter, type_printer_for<T, OutIter> Printer = template_type_printer<T>>
    constexpr OutIter print_type_to([[maybe_unused]] const priority_tag<1u>, OutIter out)
    {
        return std::ranges::copy(Printer::name(), std::move(out))
            .out;
    }

    template <typename T, print_iterator OutIter>
    OutIter print_type_to([[maybe_unused]] const priority_tag<0u>, OutIter out)
    {
        return std::ranges::copy(
                   detail::prettify_type_name(detail::type_name<T>()),
                   std::move(out))
            .out;
    }

    constexpr priority_tag<4u> maxPrintTypePriority{};

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
            return detail::type::print_type_helper<T>{}(std::move(out));
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
