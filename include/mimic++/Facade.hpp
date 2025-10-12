//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_FACADE_HPP
#define MIMICPP_FACADE_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/Mock.hpp"
#include "mimic++/config/Config.hpp"
#include "mimic++/macros/Facade.hpp"
#include "mimic++/macros/InterfaceMocking.hpp"
#include "mimic++/printing/TypePrinter.hpp"
#include "mimic++/utilities/StaticString.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <cstddef>
    #include <iterator>
    #include <tuple>
    #include <type_traits>
    #include <utility>
#endif

namespace mimicpp
{
    /**
     * \defgroup FACADE facade
     * \brief Contains utility to simplify the process of generating forwarding facade functions.
     *
     * \details
     * While this library tries avoiding macros when possible, sometimes we must not be too stubborn.
     * Making interface mocking more enjoyable is such a situation.
     * While this can, of course, be done without macros, this quickly becomes annoying, due to the necessary boilerplate code.
     * \snippet FacadeMock.cpp facade mock manual
     *
     * *mimic++* therefore introduces several macros, which helps to reduce the effort to a minimum.
     * With them, the boilerplate can be reduced to this macro invocation, which effectively does the same as before:
     * ```cpp
     * MAKE_MEMBER_METHOD(foo, void, (), override);
     * ```
     *
     * The good news is that these macros are just a thin layer around the macro-free core and can thus be easily avoided.
     * Nevertheless, *mimic++* still aims to become as macro-less as possible.
     * As soon as reflection becomes available,
     * an attempt will be made to solve this feature completely in the C++ language (hopefully with c++26, but only time will tell).
     *
     * ## Multiple inheritance
     *
     * This use-case is fully supported, without any special tricks.
     * \snippet FacadeMock.cpp facade mock multiple inheritance
     *
     * ## Mocks and variadic templates
     *
     * Due to the nature of the `mimicpp::Mock` design, it directly supports packs without any question.
     * \snippet VariadicMocks.cpp variadic mock def
     * \snippet VariadicMocks.cpp variadic mock
     *
     * The interesting part is: Do the facade macros also support variadic templates?
     *
     * Yes they do! Both handle packs correctly.
     * \snippet VariadicMocks.cpp variadic interface def
     *
     * They can then be used with arbitrary template arguments.
     * \snippet VariadicMocks.cpp variadic interface zero
     * \snippet VariadicMocks.cpp variadic interface 2
     *
     * \see A more detailed explanation about the internals can be found here
     * https://dnkpp.github.io/2024-12-15-simultaneous-pack-expansion-inside-macros/
     */
}

namespace mimicpp::facade::detail
{
    // * the generated facade implementation - lambda
    // * the generated facade implementation
    inline constexpr std::size_t facadeBaseCallDepth{2u};

    template <typename Self>
    [[nodiscard]]
    StringT generate_member_target_name(StringViewT const functionName)
    {
        StringStreamT ss{};
        mimicpp::print_type<Self>(std::ostreambuf_iterator{ss});
        ss << "::" << functionName;

        return std::move(ss).str();
    }

    template <typename Signature, typename Target>
    [[nodiscard]]
    constexpr auto&& forward_for(Target& target) noexcept
    {
        using ref = std::conditional_t<
            ValueCategory::rvalue == signature_ref_qualification_v<Signature>,
            Target&&,
            Target&>;

        return static_cast<ref>(target);
    }

    // * detail::apply::lambda
    // * detail::apply
    inline constexpr std::size_t applyCallDepth{2u};

    /**
     * \brief Applies the given args on the target.
     * \details
     * This function casts the provided target to an appropriate reference for the specified signature.
     * Additionally, it does exactly the same as `std::apply`, with the important difference that we reliably
     * know the call depth, which does vary for `std::apply` depending on the concrete implementation.
     */
    template <typename Signature, typename... Args>
    constexpr decltype(auto) apply(auto& target, std::tuple<Args...>&& args)
    {
        return [&]<std::size_t... indices>([[maybe_unused]] std::index_sequence<indices...> const) -> decltype(auto) {
            return forward_for<Signature>(target)(
                std::forward<Args>(std::get<indices>(args))...);
        }(std::index_sequence_for<Args...>{});
    }
}

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::facade
{
    template <template <typename...> typename TargetTemplate>
    struct basic_as_member
    {
        static constexpr bool is_member{true};

        template <typename... Signatures>
        using target_type = TargetTemplate<Signatures...>;

        template <typename Signature, typename... Args>
        static constexpr decltype(auto) invoke(
            auto& target,
            [[maybe_unused]] auto* const self,
            std::tuple<Args...>&& args)
        {
            return detail::apply<Signature>(target, std::move(args));
        }

        template <typename Self>
        [[nodiscard]]
        static constexpr MockSettings make_settings([[maybe_unused]] Self const* const self, StringViewT const functionName)
        {
            constexpr std::size_t skip = 1u + detail::facadeBaseCallDepth + detail::applyCallDepth;

            return MockSettings{
                .name = detail::generate_member_target_name<Self>(functionName),
                .stacktraceSkip = skip};
        }
    };

    using mock_as_member = basic_as_member<Mock>;

    template <typename Self, template <typename...> typename TargetTemplate>
    struct basic_as_member_with_this
    {
        static constexpr bool is_member{true};

        template <typename Signature, bool isConst = Constness::as_const == signature_const_qualification_v<Signature>>
        using prepend_this = signature_prepend_param_t<
            Signature,
            std::conditional_t<isConst, Self const*, Self*>>;

        template <typename... Signatures>
        using target_type = TargetTemplate<prepend_this<Signatures>...>;

        template <typename Signature, typename... Args>
        static constexpr decltype(auto) invoke(auto& target, auto* const self, std::tuple<Args...>&& args)
        {
            return detail::apply<Signature>(
                target,
                std::tuple_cat(std::make_tuple(self), std::move(args)));
        }

        [[nodiscard]]
        static constexpr MockSettings make_settings([[maybe_unused]] auto const* const self, StringViewT const functionName)
        {
            constexpr std::size_t skip = 1u + detail::facadeBaseCallDepth + detail::applyCallDepth;

            return MockSettings{
                .name = detail::generate_member_target_name<Self>(functionName),
                .stacktraceSkip = skip};
        }
    };

    template <typename Self>
    using mock_as_member_with_this = basic_as_member_with_this<Self, Mock>;
}

// These symbols are called from within "exported" macros and must thus be visible to the caller.
MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::facade::detail
{
    template <typename Traits>
    inline constexpr bool is_member_v = false;

    template <typename Traits>
        requires requires { Traits::is_member; }
    inline constexpr bool is_member_v<Traits>{Traits::is_member};

    template <auto specText>
    struct apply_normalized_specs
    {
    private:
        struct spec_info
        {
            bool hasConst{false};
            ValueCategory refQualifier = ValueCategory::any;
            bool hasNoexcept{false};
        };

        [[nodiscard]]
        static consteval auto evaluate_specs()
        {
            auto const end = std::ranges::end(specText);
            auto const find_token_begin = [&](auto const first) consteval noexcept {
                constexpr auto is_space = [](char const c) consteval noexcept {
                    return ' ' == c || '\t' == c;
                };

                return std::ranges::find_if_not(first, end, is_space);
            };

            spec_info result{};
            for (auto tokenBegin = find_token_begin(std::ranges::begin(specText));
                 tokenBegin != end;
                 tokenBegin = find_token_begin(tokenBegin))
            {
                tokenBegin = ('&' == *tokenBegin)
                               ? parse_ref_specifier(tokenBegin, end, result)
                               : parse_keyword_specifier(tokenBegin, end, result);
            }

            return result;
        }

        template <typename Iter>
        [[nodiscard]]
        static consteval Iter parse_ref_specifier(Iter first, auto const end, spec_info& out_info)
        {
            MIMICPP_ASSERT(first != end, "First must point to the first `&` character.");
            MIMICPP_ASSERT(out_info.refQualifier == ValueCategory::any, "Ref-qualifier already set.");

            if (++first != end
                && '&' == *first)
            {
                ++first;
                out_info.refQualifier = ValueCategory::rvalue;
            }
            else
            {
                out_info.refQualifier = ValueCategory::lvalue;
            }

            return first;
        }

        template <typename Iter>
        [[nodiscard]]
        static consteval Iter parse_keyword_specifier(Iter first, auto const end, spec_info& out_info)
        {
            MIMICPP_ASSERT(first != end, "First must point to the first keyword character.");

            constexpr auto is_word_continue = [](char const c) consteval noexcept {
                return ('a' <= c && c <= 'z')
                    || ('A' <= c && c <= 'Z');
            };

            auto const tokenEnd = std::ranges::find_if_not(first + 1u, end, is_word_continue);
            std::string_view const token{first, tokenEnd};
            if (constexpr std::string_view constKeyword{"const"};
                constKeyword == token)
            {
                MIMICPP_ASSERT(!out_info.hasConst, "Const-qualifier already set.");
                out_info.hasConst = true;
            }
            else if (constexpr std::string_view noexceptKeyword{"noexcept"};
                     noexceptKeyword == token)
            {
                MIMICPP_ASSERT(!out_info.hasNoexcept, "Noexcept-qualifier already set.");
                out_info.hasNoexcept = true;
            }
            else if (constexpr std::string_view overrideKeyword{"override"}, finalKeyword{"final"};
                     overrideKeyword != token && finalKeyword != token)
            {
                throw "Invalid spec";
            }

            return tokenEnd;
        }

        static constexpr auto info = evaluate_specs();

    public:
        template <typename Signature>
        [[nodiscard]]
        static consteval auto evaluate() noexcept
        {
            using sig_maybe_ref = std::conditional_t<
                ValueCategory::lvalue == info.refQualifier,
                signature_add_lvalue_ref_qualifier_t<Signature>,
                std::conditional_t<
                    ValueCategory::rvalue == info.refQualifier,
                    signature_add_rvalue_ref_qualifier_t<Signature>,
                    Signature>>;

            using sig_maybe_const = std::conditional_t<
                info.hasConst,
                signature_add_const_qualifier_t<sig_maybe_ref>,
                sig_maybe_ref>;

            using sig_maybe_noexcept = std::conditional_t<
                info.hasNoexcept,
                signature_add_noexcept_t<sig_maybe_const>,
                sig_maybe_const>;

            return std::type_identity<sig_maybe_noexcept>{};
        }

        template <typename Signature>
        using type = decltype(evaluate<Signature>())::type;
    };

    template <typename RawSignature, auto specText>
    using apply_normalized_specs_t = apply_normalized_specs<specText>::template type<RawSignature>;
}

#endif
