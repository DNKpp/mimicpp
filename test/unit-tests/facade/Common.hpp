//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_UNIT_TESTS_FACADE_COMMON_HPP
#define MIMICPP_UNIT_TESTS_FACADE_COMMON_HPP

#pragma once

#include "mimic++/Facade.hpp"
#include "mimic++/Mock.hpp"

struct TestTraits
{
    static constexpr bool is_member{true};

    template <typename... Signatures>
    using target_type = mimicpp::Mock<Signatures...>;

    template <typename Signature, typename... Args>
    static constexpr decltype(auto) invoke(
        auto& mock,
        [[maybe_unused]] auto* self,
        std::tuple<Args...>&& args)
    {
        return mimicpp::facade::detail::apply<Signature>(mock, std::move(args));
    }

    template <typename Self>
    [[nodiscard]]
    static mimicpp::MockSettings make_settings([[maybe_unused]] Self const* const self, mimicpp::StringT functionName)
    {
        return mimicpp::MockSettings{.name = std::move(functionName), .stacktraceSkip = 1u};
    }
};

#endif
