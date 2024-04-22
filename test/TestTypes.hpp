// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "mimic++/Call.hpp"
#include "mimic++/Printer.hpp"

#include "catch2/catch_test_macros.hpp"
#include "catch2/trompeloeil.hpp"

class UnwrapReferenceWrapper
{
public:
	template <typename T>
	constexpr T& operator ()(const std::reference_wrapper<T> ref) const noexcept
	{
		return ref.get();
	}
};

template <typename Signature>
class PolicyFake
{
public:
	using CallInfoT = mimicpp::call::info_for_signature_t<Signature>;
	using SubMatchT = mimicpp::call::SubMatchResult;

	bool isSatisfied{};

	[[nodiscard]]
	constexpr bool is_satisfied() const noexcept
	{
		return isSatisfied;
	}

	SubMatchT matchResult{};

	[[nodiscard]]
	constexpr SubMatchT matches(const CallInfoT& call) const noexcept
	{
		return matchResult;
	}

	static constexpr void consume(const CallInfoT& call) noexcept
	{
	}
};

template <typename Signature>
class FinalizerMock
{
public:
	using CallInfoT = mimicpp::call::info_for_signature_t<Signature>;
	using ReturnT = mimicpp::signature_return_type_t<Signature>;

	MAKE_MOCK1(finalize_call, ReturnT (const CallInfoT&));
};

template <typename Signature, typename Policy, typename Projection>
// enable, when trompeloeil fully supports movable mocks
//requires mimicpp::expectation_policy_for<
//	std::remove_cvref_t<std::invoke_result_t<Projection, Policy>>,
//	Signature>
class PolicyFacade
{
public:
	using CallT = mimicpp::call::info_for_signature_t<Signature>;
	using SubMatchT = mimicpp::call::SubMatchResult;

	Policy policy{};
	Projection projection{};

	[[nodiscard]]
	constexpr bool is_satisfied() const noexcept
	{
		return std::invoke(projection, policy)
			.is_satisfied();
	}

	[[nodiscard]]
	constexpr SubMatchT matches(const CallT& call) const noexcept
	{
		return std::invoke(projection, policy)
			.matches(call);
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

	static ReturnT finalize_call(const CallInfoT& call)
	{
		throw Exception{};
	}
};

template <typename Signature>
class PolicyMock
{
public:
	using CallInfoT = mimicpp::call::info_for_signature_t<Signature>;
	using SubMatchT = mimicpp::call::SubMatchResult;

	static constexpr bool trompeloeil_movable_mock = true;

	MAKE_CONST_MOCK0(is_satisfied, bool (), noexcept);
	MAKE_CONST_MOCK1(matches, SubMatchT (const CallInfoT&), noexcept);
	MAKE_MOCK1(consume, void (const CallInfoT&), noexcept);
};

template <typename Signature, typename Policy, typename Projection>
// enable, when trompeloeil fully supports movable mocks
//requires mimicpp::finalize_policy_for<
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

class TimesFake
{
public:
	bool isSatisfied{};

	[[nodiscard]]
	constexpr bool is_satisfied() const noexcept
	{
		return isSatisfied;
	}

	bool isSaturated{};

	[[nodiscard]]
	constexpr bool is_saturated() const noexcept
	{
		return isSaturated;
	}

	static constexpr void consume() noexcept
	{
	}
};

class TimesMock
{
public:
	MAKE_CONST_MOCK0(is_satisfied, bool (), noexcept);
	MAKE_CONST_MOCK0(is_saturated, bool (), noexcept);
	MAKE_MOCK0(consume, void ());
};

template <typename Policy, typename Projection>
class TimesFacade
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
	constexpr bool is_saturated() const noexcept
	{
		return std::invoke(projection, policy)
			.is_saturated();
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
	MAKE_CONST_MOCK1(describe, mimicpp::StringT(T));
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

	template <typename T>
	[[nodiscard]]
	constexpr bool matches(T&& target) const
	{
		return std::invoke(m_Projection, m_Matcher)
			.matches(std::forward<T>(target));
	}

	template <typename T>
	[[nodiscard]]
	constexpr mimicpp::StringT describe(T&& target) const
	{
		return std::invoke(m_Projection, m_Matcher)
			.describe(std::forward<T>(target));
	}

private:
	Matcher m_Matcher;
	Projection m_Projection;
};
