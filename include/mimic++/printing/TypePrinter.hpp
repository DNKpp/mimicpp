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

        static const RegexT omitAnonymousNamespace{R"(\(anonymous namespace\)::)"};
        name = std::regex_replace(name, omitAnonymousNamespace, "");

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

        static const RegexT omitAnonymousNamespace{"`anonymous namespace'::"};
        name = std::regex_replace(name, omitAnonymousNamespace, "");

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
            { CommonTypePrinter<T>::name() } -> std::convertible_to<StringViewT>;
        }
    constexpr OutIter print_type_to([[maybe_unused]] const priority_tag<1u>, OutIter out)
    {
        return format::format_to(
            std::move(out),
            "{}",
            CommonTypePrinter<T>::name());
    }

    template <typename T, print_iterator OutIter>
    OutIter print_type_to([[maybe_unused]] const priority_tag<0u>, OutIter out)
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
        constexpr OutIter operator()(OutIter out) const
        {
            return print_type_to<T>(maxPrintTypePriority, std::move(out));
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
    template <typename T>
    [[maybe_unused]] inline constexpr detail::PrintTypeFn<T> print_type{};
}

namespace mimicpp::detail
{
    template <>
    struct CommonTypePrinter<void>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"void"};
        }
    };

    template <>
    struct CommonTypePrinter<bool>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"bool"};
        }
    };

    template <>
    struct CommonTypePrinter<std::nullptr_t>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"std::nullptr_t"};
        }
    };

    template <>
    struct CommonTypePrinter<std::byte>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"std::byte"};
        }
    };

    template <>
    struct CommonTypePrinter<char>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"char"};
        }
    };

    template <>
    struct CommonTypePrinter<signed char>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"signed char"};
        }
    };

    template <>
    struct CommonTypePrinter<unsigned char>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"unsigned char"};
        }
    };

    template <>
    struct CommonTypePrinter<char8_t>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"char8_t"};
        }
    };

    template <>
    struct CommonTypePrinter<char16_t>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"char16_t"};
        }
    };

    template <>
    struct CommonTypePrinter<char32_t>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"char32_t"};
        }
    };

    template <>
    struct CommonTypePrinter<wchar_t>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"wchar_t"};
        }
    };

    template <>
    struct CommonTypePrinter<float>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"float"};
        }
    };

    template <>
    struct CommonTypePrinter<double>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"double"};
        }
    };

    template <>
    struct CommonTypePrinter<long double>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"long double"};
        }
    };

    template <>
    struct CommonTypePrinter<short>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"short"};
        }
    };

    template <>
    struct CommonTypePrinter<unsigned short>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"unsigned short"};
        }
    };

    template <>
    struct CommonTypePrinter<int>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"int"};
        }
    };

    template <>
    struct CommonTypePrinter<unsigned int>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"unsigned int"};
        }
    };

    template <>
    struct CommonTypePrinter<long>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"long"};
        }
    };

    template <>
    struct CommonTypePrinter<unsigned long>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"unsigned long"};
        }
    };

    template <>
    struct CommonTypePrinter<long long>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"long long"};
        }
    };

    template <>
    struct CommonTypePrinter<unsigned long long>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"unsigned long long"};
        }
    };

    // std::basic_string
    template <>
    struct CommonTypePrinter<std::string>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"std::string"};
        }
    };

    template <>
    struct CommonTypePrinter<std::u8string>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"std::u8string"};
        }
    };

    template <>
    struct CommonTypePrinter<std::u16string>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"std::u16string"};
        }
    };

    template <>
    struct CommonTypePrinter<std::u32string>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"std::u32string"};
        }
    };

    template <>
    struct CommonTypePrinter<std::wstring>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"std::wstring"};
        }
    };

    // std::basic_string_view
    template <>
    struct CommonTypePrinter<std::string_view>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"std::string_view"};
        }
    };

    template <>
    struct CommonTypePrinter<std::u8string_view>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"std::u8string_view"};
        }
    };

    template <>
    struct CommonTypePrinter<std::u16string_view>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"std::u16string_view"};
        }
    };

    template <>
    struct CommonTypePrinter<std::u32string_view>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"std::u32string_view"};
        }
    };

    template <>
    struct CommonTypePrinter<std::wstring_view>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"std::wstring_view"};
        }
    };

    // std::vector
    template <typename T>
    struct CommonTypePrinter<std::vector<T>>
    {
        [[nodiscard]]
        static StringViewT name()
        {
            static const StringT str = format::format(
                "std::vector<{}>",
                mimicpp::print_type<T>());

            return str;
        }
    };

    template <typename T>
    struct CommonTypePrinter<std::pmr::vector<T>>
    {
        [[nodiscard]]
        static StringViewT name()
        {
            static const StringT str = format::format(
                "std::pmr::vector<{}>",
                mimicpp::print_type<T>());

            return str;
        }
    };
}

#endif
