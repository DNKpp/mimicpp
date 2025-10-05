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

    template <typename Traits>
    inline constexpr bool is_member_v = false;

    template <typename Traits>
        requires requires { Traits::is_member; }
    inline constexpr bool is_member_v<Traits>{Traits::is_member};
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
        static MockSettings make_settings([[maybe_unused]] Self const* const self, StringViewT const functionName)
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
        static MockSettings make_settings([[maybe_unused]] auto const* const self, StringViewT const functionName)
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
    template <auto specText>
    struct apply_normalized_specs
    {
        [[nodiscard]]
        static consteval auto evaluate_specs()
        {
            constexpr std::string_view constKeyword{"const"};
            constexpr std::string_view noexceptKeyword{"noexcept"};
            constexpr std::string_view overrideKeyword{"override"};
            constexpr std::string_view finalKeyword{"final"};

            auto const end = std::ranges::end(specText);
            auto const find_token_begin = [&](auto const first) noexcept {
                constexpr auto is_space = [](char const c) noexcept {
                    return ' ' == c || '\t' == c;
                };

                return std::ranges::find_if_not(first, end, is_space);
            };

            struct spec_info
            {
                bool hasConst{false};
                ValueCategory refQualifier = ValueCategory::any;
                bool hasNoexcept{false};
            };

            spec_info result{};
            for (auto tokenBegin = find_token_begin(std::ranges::begin(specText));
                 tokenBegin != end;
                 tokenBegin = find_token_begin(tokenBegin))
            {
                if ('&' == *tokenBegin)
                {
                    MIMICPP_ASSERT(result.refQualifier == ValueCategory::any, "Ref-qualifier already set.");
                    if (++tokenBegin != end
                        && '&' == *tokenBegin)
                    {
                        ++tokenBegin;
                        result.refQualifier = ValueCategory::rvalue;
                    }
                    else
                    {
                        result.refQualifier = ValueCategory::lvalue;
                    }
                }
                else
                {
                    constexpr auto is_word_continue = [](char const c) noexcept {
                        return ('a' <= c && c <= 'z')
                            || ('A' <= c && c <= 'Z');
                    };

                    auto const tokenEnd = std::ranges::find_if_not(tokenBegin, end, is_word_continue);
                    std::string_view const token{tokenBegin, tokenEnd};
                    if (constKeyword == token)
                    {
                        MIMICPP_ASSERT(!result.hasConst, "Const-qualifier already set.");
                        result.hasConst = true;
                    }
                    else if (noexceptKeyword == token)
                    {
                        MIMICPP_ASSERT(!result.hasNoexcept, "Noexcept-qualifier already set.");
                        result.hasNoexcept = true;
                    }
                    else if (overrideKeyword != token && finalKeyword != token)
                    {
                        throw std::runtime_error{"Invalid spec"};
                    }

                    tokenBegin = tokenEnd;
                }
            }

            return result;
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
