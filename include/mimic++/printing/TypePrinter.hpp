//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_PRINTING_TYPE_PRINTER_HPP
#define MIMICPP_PRINTING_TYPE_PRINTER_HPP

#include "mimic++/Printer.hpp"

#include <cassert>
#include <concepts>
#include <functional>
#include <iterator>
#include <regex>
#include <typeinfo>
#include <utility>

namespace mimicpp
{
    using RegexT = std::regex;
}

#ifdef __GNUC__
    #define MIMICPP_DETAIL_IS_GCC
#endif

namespace mimicpp::detail
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
                assert(1 == match.size());
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

#ifdef MIMICPP_DETAIL_IS_GCC

#include <cstdlib>
#include <cxxabi.h>

namespace mimicpp::detail
{
    template <typename T>
    StringT type_name()
    {
        StringT name{typeid(T).name()};

        // see: https://gcc.gnu.org/onlinedocs/libstdc++/manual/ext_demangling.html
        int status{};
        char* rawName = abi::__cxa_demangle(name.data(), nullptr, nullptr, &status);
        if (0 == status)
        {
            name = StringT{rawName};
        }

        std::free(rawName);
        return name;
    }

    inline StringT prettify_type_name(StringT name)
    {
        static const RegexT unifyClosingAngleBrackets{R"(\>\s*\>)"};
        name = regex_replace_all(name, unifyClosingAngleBrackets, ">>");

        static const RegexT handleStdNamespace{R"(std::__cxx11::)"};
        name = std::regex_replace(name, handleStdNamespace, "std::");

        return name;
    }
}

#else

namespace mimicpp::detail
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

        return name;
    }
}

#endif

namespace mimicpp::detail
{
    template <typename T>
    struct CommonTypePrinter;

    template <typename T, print_iterator OutIter>
        requires requires {
            typename CommonTypePrinter<T>;
            { CommonTypePrinter<T>::value } -> std::convertible_to<StringViewT>;
        }
    OutIter print_type([[maybe_unused]] const priority_tag<1u>, OutIter out)
    {
        return format::format_to(
            std::move(out),
            "{}",
            CommonTypePrinter<T>::value);
    }

    template <typename T, print_iterator OutIter>
    OutIter print_type([[maybe_unused]] const priority_tag<0u>, OutIter out)
    {
        static const StringT name = detail::prettify_type_name(
            detail::type_name<T>());

        return format::format_to(
            std::move(out),
            "{}",
            name);
    }

    constexpr priority_tag<1u> maxPrintTypePriority{};

    template <typename T>
    class PrintTypeFn
    {
    public:
        template <print_iterator OutIter>
        OutIter operator()(OutIter out) const
        {
            return print_type<T>(maxPrintTypePriority, std::move(out));
        }

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
    template <typename T>
    [[maybe_unused]] inline constexpr detail::PrintTypeFn<T> print_type{};
}

namespace mimicpp::detail
{
    template <>
    struct CommonTypePrinter<void>
    {
        static constexpr StringViewT value{"void"};
    };

    template <>
    struct CommonTypePrinter<bool>
    {
        static constexpr StringViewT value{"bool"};
    };

    template <>
    struct CommonTypePrinter<std::nullptr_t>
    {
        static constexpr StringViewT value{"std::nullptr_t"};
    };

    template <>
    struct CommonTypePrinter<std::byte>
    {
        static constexpr StringViewT value{"std::byte"};
    };

    template <>
    struct CommonTypePrinter<char>
    {
        static constexpr StringViewT value{"char"};
    };

    template <>
    struct CommonTypePrinter<signed char>
    {
        static constexpr StringViewT value{"signed char"};
    };

    template <>
    struct CommonTypePrinter<unsigned char>
    {
        static constexpr StringViewT value{"unsigned char"};
    };

    template <>
    struct CommonTypePrinter<char8_t>
    {
        static constexpr StringViewT value{"char8_t"};
    };

    template <>
    struct CommonTypePrinter<char16_t>
    {
        static constexpr StringViewT value{"char16_t"};
    };

    template <>
    struct CommonTypePrinter<char32_t>
    {
        static constexpr StringViewT value{"char32_t"};
    };

    template <>
    struct CommonTypePrinter<wchar_t>
    {
        static constexpr StringViewT value{"wchar_t"};
    };

    template <>
    struct CommonTypePrinter<float>
    {
        static constexpr StringViewT value{"float"};
    };

    template <>
    struct CommonTypePrinter<double>
    {
        static constexpr StringViewT value{"double"};
    };

    template <>
    struct CommonTypePrinter<long double>
    {
        static constexpr StringViewT value{"long double"};
    };

    template <>
    struct CommonTypePrinter<short>
    {
        static constexpr StringViewT value{"short"};
    };

    template <>
    struct CommonTypePrinter<unsigned short>
    {
        static constexpr StringViewT value{"unsigned short"};
    };

    template <>
    struct CommonTypePrinter<int>
    {
        static constexpr StringViewT value{"int"};
    };

    template <>
    struct CommonTypePrinter<unsigned int>
    {
        static constexpr StringViewT value{"unsigned int"};
    };

    template <>
    struct CommonTypePrinter<long>
    {
        static constexpr StringViewT value{"long"};
    };

    template <>
    struct CommonTypePrinter<unsigned long>
    {
        static constexpr StringViewT value{"unsigned long"};
    };

    template <>
    struct CommonTypePrinter<long long>
    {
        static constexpr StringViewT value{"long long"};
    };

    template <>
    struct CommonTypePrinter<unsigned long long>
    {
        static constexpr StringViewT value{"unsigned long long"};
    };
}

#endif
