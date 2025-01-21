//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_PRINTING_TYPE_PRINTER_HPP
#define MIMICPP_PRINTING_TYPE_PRINTER_HPP

#include "mimic++/Printer.hpp"

#include <array>
#include <cassert>
#include <concepts>
#include <functional>
#include <iterator>
#include <regex>
#include <span>
#include <string>
#include <string_view>
#include <typeinfo>
#include <utility>

namespace mimicpp
{
    using RegexT = std::regex;
}

#ifdef __GNUC__
    #define MIMICPP_DETAIL_IS_GCC
#endif

#ifdef _LIBCPP_VERSION
    #define MIMICPP_DETAIL_IS_LIBCXX
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

    #ifdef MIMICPP_DETAIL_IS_LIBCXX
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
    constexpr OutIter print_type_to([[maybe_unused]] const priority_tag<3u>, OutIter out)
    {
        return format::format_to(
            std::move(out),
            "{}",
            CommonTypePrinter<T>::name());
    }

    template <typename T>
    struct SignatureTypePrinter;

    template <typename T, print_iterator OutIter>
        requires requires {
            typename SignatureTypePrinter<T>;
            { SignatureTypePrinter<T>::name() } -> std::convertible_to<StringViewT>;
        }
    constexpr OutIter print_type_to([[maybe_unused]] const priority_tag<2u>, OutIter out)
    {
        return format::format_to(
            std::move(out),
            "{}",
            SignatureTypePrinter<T>::name());
    }

    template <typename T>
    struct TemplateTypePrinter;

    template <typename T, print_iterator OutIter>
        requires requires {
            typename TemplateTypePrinter<T>;
            { TemplateTypePrinter<T>::name() } -> std::convertible_to<StringViewT>;
        }
    constexpr OutIter print_type_to([[maybe_unused]] const priority_tag<1u>, OutIter out)
    {
        return format::format_to(
            std::move(out),
            "{}",
            TemplateTypePrinter<T>::name());
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

    constexpr priority_tag<3u> maxPrintTypePriority{};

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
    struct print_type_helper<volatile T>
    {
        template <print_iterator OutIter>
        [[nodiscard]]
        constexpr OutIter operator()(OutIter out) const
        {
            return std::ranges::copy(
                       StringViewT{" volatile"},
                       print_type_helper<T>{}(std::move(out)))
                .out;
        }
    };

    template <typename T>
    struct print_type_helper<const T>
    {
        template <print_iterator OutIter>
        [[nodiscard]]
        constexpr OutIter operator()(OutIter out) const
        {
            return std::ranges::copy(
                       StringViewT{" const"},
                       print_type_helper<T>{}(std::move(out)))
                .out;
        }
    };

    template <typename T>
    struct print_type_helper<const volatile T>
    {
        template <print_iterator OutIter>
        [[nodiscard]]
        constexpr OutIter operator()(OutIter out) const
        {
            return std::ranges::copy(
                       StringViewT{" const volatile"},
                       print_type_helper<T>{}(std::move(out)))
                .out;
        }
    };

    template <typename T>
    struct print_type_helper<T&>
    {
        template <print_iterator OutIter>
        [[nodiscard]]
        constexpr OutIter operator()(OutIter out) const
        {
            return std::ranges::fill_n(
                print_type_helper<T>{}(std::move(out)),
                1u,
                '&');
        }
    };

    template <typename T>
    struct print_type_helper<T&&>
    {
        template <print_iterator OutIter>
        [[nodiscard]]
        constexpr OutIter operator()(OutIter out) const
        {
            return std::ranges::fill_n(
                print_type_helper<T>{}(std::move(out)),
                2u,
                '&');
        }
    };

    template <typename T>
    struct print_type_helper<T*>
    {
        template <print_iterator OutIter>
        [[nodiscard]]
        constexpr OutIter operator()(OutIter out) const
        {
            return std::ranges::fill_n(
                print_type_helper<T>{}(std::move(out)),
                1u,
                '*');
        }
    };

    template <typename T>
    class PrintTypeFn
    {
    public:
        template <print_iterator OutIter>
        constexpr OutIter operator()(OutIter out) const
        {
            return print_type_helper<T>{}(std::move(out));
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
    template <satisfies<std::is_function> Signature>
    struct SignatureTypePrinter<Signature>
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

            using param_list_t = signature_param_list_t<Signature>;
            if constexpr (0u < param_list_t::size)
            {
                std::invoke(
                    [&]<typename First, typename... Params>([[maybe_unused]] const type_list<First, Params...>) {
                        mimicpp::print_type<First>(std::ostreambuf_iterator{out});
                        ((out << ", ", mimicpp::print_type<Params>(std::ostreambuf_iterator{out})), ...);
                    },
                    param_list_t{});
            }

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

    template <template <typename...> typename Template, typename First, typename... Others>
    struct TemplateTypePrinter<Template<First, Others...>>
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
            StringT fullName = detail::prettify_type_name(
                detail::type_name<Template<First, Others...>>());
            fullName.erase(std::ranges::find(fullName, '<'), fullName.cend());

            StringStreamT out{};
            out << fullName << '<' << mimicpp::print_type<First>();
            ((out << ", " << mimicpp::print_type<Others>()), ...);
            out << '>';

            return std::move(out).str();
        }
    };

    // required for msvc
    template <>
    struct CommonTypePrinter<long long>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"long long"};
        }
    };

    // required for msvc
    template <>
    struct CommonTypePrinter<unsigned long long>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"unsigned long long"};
        }
    };

    // required for gcc
    template <>
    struct CommonTypePrinter<std::nullptr_t>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"std::nullptr_t"};
        }
    };

    // required for AppleClang
    template <>
    struct CommonTypePrinter<char8_t>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"char8_t"};
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

    // std::array
    template <typename T, std::size_t n>
    struct CommonTypePrinter<std::array<T, n>>
    {
        [[nodiscard]]
        static StringViewT name()
        {
            static const StringT str = format::format(
                "std::array<{}, {}>",
                mimicpp::print_type<T>(),
                n);

            return str;
        }
    };
}

#endif
