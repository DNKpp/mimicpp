//          Copyright Dominic (DNKpp) Koepke 2024-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "Common.hpp"
#include "mimic++/TypeTraits.hpp"

TEMPLATE_TEST_CASE_SIG(
    "signature_arity yields the parameter count.",
    "[type_traits]",
    ((typename Return, typename... Args), Return, Args...),
    TEST_SIGNATURE_COLLECTION)
{
    constexpr auto with_expected_arity = std::bind_front(std::equal_to{}, sizeof...(Args));

    CHECK_THAT(
        std::to_array({
            mimicpp::signature_arity<Return(Args...)>::value,
            mimicpp::signature_arity_v<Return(Args...)>,
            mimicpp::signature_arity<Return(Args...) noexcept>::value,
            mimicpp::signature_arity_v<Return(Args...) noexcept>,

            mimicpp::signature_arity<Return(Args...) const>::value,
            mimicpp::signature_arity_v<Return(Args...) const>,
            mimicpp::signature_arity<Return(Args...) const noexcept>::value,
            mimicpp::signature_arity_v<Return(Args...) const noexcept>,

            mimicpp::signature_arity<Return(Args...)&>::value,
            mimicpp::signature_arity_v<Return(Args...)&>,
            mimicpp::signature_arity < Return(Args...) & noexcept > ::value,
            mimicpp::signature_arity_v < Return(Args...) & noexcept >,

            mimicpp::signature_arity<Return(Args...) const&>::value,
            mimicpp::signature_arity_v<Return(Args...) const&>,
            mimicpp::signature_arity < Return(Args...) const & noexcept > ::value,
            mimicpp::signature_arity_v < Return(Args...) const & noexcept >,

            mimicpp::signature_arity<Return(Args...) &&>::value,
            mimicpp::signature_arity_v<Return(Args...) &&>,
            mimicpp::signature_arity < Return(Args...) && noexcept > ::value,
            mimicpp::signature_arity_v < Return(Args...) && noexcept >,

            mimicpp::signature_arity<Return(Args...) const&&>::value,
            mimicpp::signature_arity_v<Return(Args...) const&&>,
            mimicpp::signature_arity < Return(Args...) const && noexcept > ::value,
            mimicpp::signature_arity_v < Return(Args...) const && noexcept >,
        }),
        Catch::Matchers::AllMatch(Catch::Matchers::Predicate<std::size_t>(with_expected_arity)));
}
