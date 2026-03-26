//          Copyright Dominic (DNKpp) Koepke 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_EXPECTATION_COMMON_HPP
#define MIMICPP_EXPECTATION_COMMON_HPP

#pragma once

#include "mimic++/Call.hpp"
#include "mimic++/Fwd.hpp"
#include "mimic++/config/Config.hpp"
#include "mimic++/reporting/ExpectationReport.hpp"
#include "mimic++/utilities/Concepts.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <concepts>
    #include <optional>
    #include <type_traits>
    #include <utility>
#endif

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::expectation
{
    /**
     * \brief Determines whether the given type satisfies the requirements of an expectation-policy for the given signature.
     * \ingroup EXPECTATION
     */
    template <typename T, typename Signature>
    concept expectation_policy_for = std::is_move_constructible_v<T>
                                  && std::is_destructible_v<T>
                                  && std::same_as<T, std::remove_cvref_t<T>>
                                  && requires(T& policy, call::info_for_signature_t<Signature> const& info) {
                                         { std::as_const(policy).is_satisfied() } noexcept -> util::boolean_testable;
                                         { std::as_const(policy).matches(info) } -> util::boolean_testable;
                                         { std::as_const(policy).describe() } -> util::explicitly_convertible_to<std::optional<StringT>>;
                                         { policy.consume(info) };
                                     };

    /**
     * \brief Determines whether the given type satisfies the requirements of a finalize-policy for the given signature.
     * \ingroup EXPECTATION
     */
    template <typename T, typename Signature>
    concept finalize_policy_for = std::is_move_constructible_v<T>
                               && std::is_destructible_v<T>
                               && std::same_as<T, std::remove_cvref_t<T>>
                               && requires(T& policy, call::info_for_signature_t<Signature> const& info) {
                                      { policy.finalize_call(info) } -> std::convertible_to<signature_return_type_t<Signature>>;
                                  };

    /**
     * \brief Determines whether the given type satisfies the requirements of a control-policy.
     * \ingroup EXPECTATION
     */
    template <typename T>
    concept control_policy = std::is_move_constructible_v<T>
                          && std::is_destructible_v<T>
                          && std::same_as<T, std::remove_cvref_t<T>>
                          && requires(T& policy) {
                                 { std::as_const(policy).is_satisfied() } noexcept -> util::boolean_testable;
                                 { std::as_const(policy).state() } -> std::convertible_to<reporting::control_state_t>;
                                 policy.consume();
                             };
}

#endif
