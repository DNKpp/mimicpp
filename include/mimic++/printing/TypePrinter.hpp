//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_PRINTING_TYPE_PRINTER_HPP
#define MIMICPP_PRINTING_TYPE_PRINTER_HPP

#include "mimic++/Printer.hpp"
#include "mimic++/Utility.hpp"

#include <algorithm>
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

#ifdef _LIBCPP_VERSION
    #define MIMICPP_DETAIL_IS_LIBCXX
#endif

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

#ifdef MIMICPP_DETAIL_IS_GCC

    #include <cstdlib>
    #include <cxxabi.h>

namespace mimicpp::printing::detail
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

namespace mimicpp::printing::detail
{
    template <typename T>
    struct common_type_printer;

    template <typename T, print_iterator OutIter>
        requires requires
        {
            typename common_type_printer<T>;
            { common_type_printer<T>::name() } -> std::convertible_to<StringViewT>;
        }
    constexpr OutIter print_type_to([[maybe_unused]] const priority_tag<3u>, OutIter out)
    {
        return format::format_to(
            std::move(out),
            "{}",
            common_type_printer<T>::name());
    }

    template <typename T>
    struct SignatureTypePrinter;

    template <typename T, print_iterator OutIter>
        requires requires
        {
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
    struct template_type_printer;

    template <typename T, print_iterator OutIter>
        requires requires
        {
            typename template_type_printer<T>;
            { template_type_printer<T>::name() } -> std::convertible_to<StringViewT>;
        }
    constexpr OutIter print_type_to([[maybe_unused]] const priority_tag<1u>, OutIter out)
    {
        return format::format_to(
            std::move(out),
            "{}",
            template_type_printer<T>::name());
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
            return std::ranges::copy(
                    StringViewT{" volatile"},
                    print_type_helper<T>{}(std::move(out)))
                .out;
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
            return std::ranges::copy(
                    StringViewT{" const"},
                    print_type_helper<T>{}(std::move(out)))
                .out;
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
    struct print_type_helper<T[]>
    {
        template <print_iterator OutIter>
        [[nodiscard]]
        constexpr OutIter operator()(OutIter out) const
        {
            return std::ranges::copy(
                    StringViewT{"[]"},
                    print_type_helper<T>{}(std::move(out)))
                .out;
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
    [[maybe_unused]] inline constexpr printing::detail::PrintTypeFn<T> print_type{};
}

namespace mimicpp::printing::detail
{
    template <typename Out, typename... Ts>
    constexpr Out& print_separated(Out& out, const StringViewT separator, const type_list<Ts...> ts)
    {
        if constexpr (0u < sizeof...(Ts))
        {
            std::invoke(
                [&]<typename First, typename... Others>([[maybe_unused]] const type_list<First, Others...>)
                {
                    mimicpp::print_type<First>(std::ostreambuf_iterator{out});
                    ((out << separator, mimicpp::print_type<Others>(std::ostreambuf_iterator{out})), ...);
                },
                ts);
        }

        return out;
    }

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

    template <typename T>
    [[nodiscard]]
    StringT pretty_template_name()
    {
        StringT name = detail::prettify_type_name(detail::type_name<T>());
        name.erase(std::ranges::find(name, '<'), name.cend());

        return name;
    }

    template <typename NameGenerator>
    struct basic_template_type_printer
    {
        [[nodiscard]]
        static StringViewT name()
        {
            static const StringT name = std::invoke(NameGenerator{});
            return name;
        }
    };

    template <typename Type, template <typename...> typename Template, typename LeadingArgList>
    struct is_default_arg_for
        : public std::false_type
    {
    };

    template <typename Type, template <typename...> typename Template, typename... LeadingArgs>
        requires requires { typename Template<LeadingArgs...>; }
    struct is_default_arg_for<Type, Template, type_list<LeadingArgs...>>
        : public std::bool_constant<
            std::same_as<
                Template<LeadingArgs...>,
                Template<LeadingArgs..., Type>>>
    {
    };

    template <typename Type, template <typename...> typename Template, typename LeadingArgList>
    concept default_arg_for = is_default_arg_for<Type, Template, LeadingArgList>::value;

    template <typename List>
    struct type_list_pop_back_if_can
        : public type_list_pop_back<List>
    {
    };

    template <>
    struct type_list_pop_back_if_can<type_list<>>
    {
        using type = type_list<>;
    };

    template <
        typename ArgList,
        template <typename...> typename Template,
        typename PopBack = type_list_pop_back_if_can<ArgList>>
    struct drop_default_args_for_impl
    {
        using type = ArgList;
    };

    template <typename ArgList, template <typename...> typename Template, typename PopBack>
        requires requires { typename PopBack::popped; }
        && default_arg_for<typename PopBack::popped, Template, typename PopBack::type>
    struct drop_default_args_for_impl<ArgList, Template, PopBack>
        : public drop_default_args_for_impl<typename PopBack::type, Template>
    {
    };

    template <template <typename...> typename Template, typename ArgList>
    struct drop_default_args_for
        : public drop_default_args_for_impl<ArgList, Template>
    {
    };

    template <template <typename...> typename Template, typename... Ts>
    struct template_type_printer<Template<Ts...>>
        : public basic_template_type_printer<
            decltype([]
            {
                using ArgList = typename drop_default_args_for<Template, type_list<Ts...>>::type;
                using Type = type_list_populate_t<Template, ArgList>;
                const StringT templateName = pretty_template_name<Type>();

                StringStreamT out{};
                out << templateName << '<';
                print_separated(out, ", ", ArgList{});
                out << '>';

                return std::move(out).str();
            })>
    {
    };
}

#endif
