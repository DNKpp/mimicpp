//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "mimic++/Call.hpp"
#include "mimic++/reporting/ExpectationReport.hpp"

#include "TrompeloeilExt.hpp"

#include <array>
#include <variant>

inline constexpr std::array refQualifiers{
    mimicpp::ValueCategory::lvalue,
    mimicpp::ValueCategory::rvalue,
    mimicpp::ValueCategory::any};
inline constexpr std::array constQualifiers{
    mimicpp::Constness::non_const,
    mimicpp::Constness::as_const,
    mimicpp::Constness::any};

class UnwrapReferenceWrapper
{
public:
    template <typename T>
    constexpr T& operator()(const std::reference_wrapper<T> ref) const noexcept
    {
        return ref.get();
    }
};

template <typename Signature>
class PolicyFake
{
public:
    using CallInfoT = mimicpp::call::info_for_signature_t<Signature>;

    bool isSatisfied{};

    [[nodiscard]]
    constexpr bool is_satisfied() const noexcept
    {
        return isSatisfied;
    }

    bool matchResult{};

    [[nodiscard]]
    constexpr bool matches([[maybe_unused]] const CallInfoT& call) const noexcept
    {
        return matchResult;
    }

    mimicpp::StringT description{};

    [[nodiscard]]
    mimicpp::StringT describe() const
    {
        return description;
    }

    static constexpr void consume([[maybe_unused]] const CallInfoT& call) noexcept
    {
    }
};

template <typename Signature>
class FinalizerMock
{
public:
    using CallInfoT = mimicpp::call::info_for_signature_t<Signature>;
    using ReturnT = mimicpp::signature_return_type_t<Signature>;

    MAKE_MOCK1(finalize_call, ReturnT(const CallInfoT&));
};

template <typename Signature, typename Policy, typename Projection>
// enable, when trompeloeil fully supports movable mocks
// requires mimicpp::expectation_policy_for<
//	std::remove_cvref_t<std::invoke_result_t<Projection, Policy>>,
//	Signature>
class PolicyFacade
{
public:
    using CallT = mimicpp::call::info_for_signature_t<Signature>;

    Policy policy;
    Projection projection{};

    [[nodiscard]]
    constexpr bool is_satisfied() const noexcept
    {
        return std::invoke(projection, policy)
            .is_satisfied();
    }

    [[nodiscard]]
    constexpr bool matches(const CallT& call) const noexcept
    {
        return std::invoke(projection, policy)
            .matches(call);
    }

    [[nodiscard]]
    mimicpp::StringT describe() const
    {
        return std::invoke(projection, policy)
            .describe();
    }

    constexpr void consume(const CallT& call) noexcept
    {
        std::invoke(projection, policy)
            .consume(call);
    }
};

template <typename Signature>
class FinalizerFake
{
public:
    using CallInfoT = mimicpp::call::info_for_signature_t<Signature>;
    using ReturnT = mimicpp::signature_return_type_t<Signature>;

    class Exception
    {
    };

    static ReturnT finalize_call([[maybe_unused]] const CallInfoT& call)
    {
        throw Exception{};
    }
};

template <typename Signature>
class PolicyMock
{
public:
    using CallInfoT = mimicpp::call::info_for_signature_t<Signature>;

    static constexpr bool trompeloeil_movable_mock = true;

    MAKE_CONST_MOCK0(is_satisfied, bool(), noexcept);
    MAKE_CONST_MOCK1(matches, bool(const CallInfoT&), noexcept);
    MAKE_CONST_MOCK0(describe, mimicpp::StringT());
    MAKE_MOCK1(consume, void(const CallInfoT&), noexcept);
};

template <typename Signature, typename Policy, typename Projection>
// enable, when trompeloeil fully supports movable mocks
// requires mimicpp::finalize_policy_for<
//	std::remove_cvref_t<std::invoke_result_t<Projection, Policy>>,
//	Signature>
class FinalizerFacade
{
public:
    using CallInfoT = mimicpp::call::info_for_signature_t<Signature>;
    using ReturnT = mimicpp::signature_return_type_t<Signature>;

    Policy policy{};
    Projection projection{};

    [[nodiscard]]
    constexpr ReturnT finalize_call(const CallInfoT& call)
    {
        return std::invoke(projection, policy)
            .finalize_call(call);
    }
};

class ControlPolicyFake
{
public:
    bool isSatisfied{};

    [[nodiscard]]
    constexpr bool is_satisfied() const noexcept
    {
        return isSatisfied;
    }

    mimicpp::reporting::control_state_t stateData{};

    [[nodiscard]]
    mimicpp::reporting::control_state_t state() const
    {
        return stateData;
    }

    static constexpr void consume() noexcept
    {
    }
};

class ControlPolicyMock
{
public:
    MAKE_CONST_MOCK0(is_satisfied, bool(), noexcept);
    MAKE_CONST_MOCK0(describe_state, std::optional<mimicpp::StringT>());
    MAKE_CONST_MOCK0(state, mimicpp::reporting::control_state_t());
    MAKE_MOCK0(consume, void());
};

template <typename Policy, typename Projection>
class ControlPolicyFacade
{
public:
    Policy policy{};
    Projection projection{};

    [[nodiscard]]
    constexpr bool is_satisfied() const noexcept
    {
        return std::invoke(projection, policy)
            .is_satisfied();
    }

    [[nodiscard]]
    mimicpp::reporting::control_state_t state() const
    {
        return std::invoke(projection, policy)
            .state();
    }

    constexpr void consume() noexcept
    {
        return std::invoke(projection, policy)
            .consume();
    }
};

template <typename T>
class MatcherMock
{
public:
    MAKE_CONST_MOCK1(matches, bool(T));
    MAKE_CONST_MOCK0(describe, mimicpp::StringT());
};

template <typename Matcher, typename Projection>
class [[maybe_unused]] MatcherFacade
{
public:
    [[nodiscard]]
    MatcherFacade(Matcher matcher, Projection projection)
        : m_Matcher{std::move(matcher)},
          m_Projection{std::move(projection)}
    {
    }

    template <typename... Args>
    [[nodiscard]]
    constexpr bool matches(Args&&... args) const
    {
        return std::invoke(m_Projection, m_Matcher)
            .matches(std::forward<Args>(args)...);
    }

    [[nodiscard]]
    constexpr mimicpp::StringT describe() const
    {
        return std::invoke(m_Projection, m_Matcher)
            .describe();
    }

private:
    Matcher m_Matcher;
    Projection m_Projection;
};

template <std::equality_comparable Value>
class VariantEqualsMatcher final
    : public Catch::Matchers::MatcherGenericBase
{
public:
    [[nodiscard]]
    explicit constexpr VariantEqualsMatcher(Value value)
        : m_Value{std::move(value)}
    {
    }

    template <typename... Alternatives>
    [[nodiscard]]
    constexpr bool match(const std::variant<Alternatives...>& other) const
        requires requires { { std::holds_alternative<Value>(other) } -> std::convertible_to<bool>; }
    {
        return std::holds_alternative<Value>(other)
            && m_Value == std::get<Value>(other);
    }

    [[nodiscard]]
    std::string describe() const override
    {
        return std::string{"Variant state equals: "}
             + mimicpp::print_type<Value>()
             + ": "
             + Catch::Detail::stringify(m_Value);
    }

private:
    Value m_Value;
};

template <typename Value>
[[nodiscard]]
constexpr auto variant_equals(Value&& value)
{
    return VariantEqualsMatcher<std::remove_cvref_t<Value>>{
        std::forward<Value>(value)};
}

class FakeSequenceStrategy
{
public:
    [[nodiscard, maybe_unused]]
    constexpr int operator()(const auto id, [[maybe_unused]] const int cursor) const noexcept
    {
        return static_cast<int>(id);
    }
};
