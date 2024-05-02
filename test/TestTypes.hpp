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

	Policy policy;
	Projection projection;

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

	bool isApplicable{true};

	[[nodiscard]]
	constexpr bool is_applicable() const noexcept
	{
		return isApplicable;
	}

	static constexpr void consume() noexcept
	{
	}
};

class TimesMock
{
public:
	MAKE_CONST_MOCK0(is_satisfied, bool (), noexcept);
	MAKE_CONST_MOCK0(is_applicable, bool (), noexcept);
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
	constexpr bool is_applicable() const noexcept
	{
		return std::invoke(projection, policy)
			.is_applicable();
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

	template <typename T>
	[[nodiscard]]
	constexpr bool matches(T&& target) const
	{
		return std::invoke(m_Projection, m_Matcher)
			.matches(std::forward<T>(target));
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

template <typename Return, typename... Params>
class InvocableMockBase;

template <typename Return, typename... Params>
	requires (0u == sizeof...(Params))
class InvocableMockBase<Return, Params...>
{
public:
	MAKE_MOCK0(Invoke, Return(Params...));
	MAKE_CONST_MOCK0(Invoke, Return(Params...));
};

template <typename Return, typename... Params>
	requires (1u == sizeof...(Params))
class InvocableMockBase<Return, Params...>
{
public:
	MAKE_MOCK1(Invoke, Return(Params...));
	MAKE_CONST_MOCK1(Invoke, Return(Params...));
};

template <typename Return, typename... Params>
	requires (2u == sizeof...(Params))
class InvocableMockBase<Return, Params...>
{
public:
	MAKE_MOCK2(Invoke, Return(Params...));
	MAKE_CONST_MOCK2(Invoke, Return(Params...));
};

template <typename Return, typename... Params>
	requires (3u == sizeof...(Params))
class InvocableMockBase<Return, Params...>
{
public:
	MAKE_MOCK3(Invoke, Return(Params...));
	MAKE_CONST_MOCK3(Invoke, Return(Params...));
};

template <typename Return, typename... Params>
	requires (4u == sizeof...(Params))
class InvocableMockBase<Return, Params...>
{
public:
	MAKE_MOCK4(Invoke, Return(Params...));
	MAKE_CONST_MOCK4(Invoke, Return(Params...));
};

template <typename Return, typename... Params>
	requires (5u == sizeof...(Params))
class InvocableMockBase<Return, Params...>
{
public:
	MAKE_MOCK5(Invoke, Return(Params...));
	MAKE_CONST_MOCK5(Invoke, Return(Params...));
};

template <typename Return, typename... Params>
	requires (6u == sizeof...(Params))
class InvocableMockBase<Return, Params...>
{
public:
	MAKE_MOCK6(Invoke, Return(Params...));
	MAKE_CONST_MOCK6(Invoke, Return(Params...));
};

template <typename Return, typename... Params>
	requires (7u == sizeof...(Params))
class InvocableMockBase<Return, Params...>
{
public:
	MAKE_MOCK7(Invoke, Return(Params...));
	MAKE_CONST_MOCK7(Invoke, Return(Params...));
};

template <typename Return, typename... Params>
	requires (8u == sizeof...(Params))
class InvocableMockBase<Return, Params...>
{
public:
	MAKE_MOCK8(Invoke, Return(Params...));
	MAKE_CONST_MOCK8(Invoke, Return(Params...));
};

template <typename Return, typename... Params>
	requires (9u == sizeof...(Params))
class InvocableMockBase<Return, Params...>
{
public:
	MAKE_MOCK9(Invoke, Return(Params...));
	MAKE_CONST_MOCK9(Invoke, Return(Params...));
};

template <typename Return, typename... Params>
	requires (10u == sizeof...(Params))
class InvocableMockBase<Return, Params...>
{
public:
	MAKE_MOCK10(Invoke, Return(Params...));
	MAKE_CONST_MOCK10(Invoke, Return(Params...));
};

template <typename Return, typename... Params>
	requires (11u == sizeof...(Params))
class InvocableMockBase<Return, Params...>
{
public:
	MAKE_MOCK11(Invoke, Return(Params...));
	MAKE_CONST_MOCK11(Invoke, Return(Params...));
};

template <typename Return, typename... Params>
	requires (12u == sizeof...(Params))
class InvocableMockBase<Return, Params...>
{
public:
	MAKE_MOCK12(Invoke, Return(Params...));
	MAKE_CONST_MOCK12(Invoke, Return(Params...));
};

template <typename Return, typename... Params>
	requires (13u == sizeof...(Params))
class InvocableMockBase<Return, Params...>
{
public:
	MAKE_MOCK13(Invoke, Return(Params...));
	MAKE_CONST_MOCK13(Invoke, Return(Params...));
};

template <typename Return, typename... Params>
	requires (14u == sizeof...(Params))
class InvocableMockBase<Return, Params...>
{
public:
	MAKE_MOCK14(Invoke, Return(Params...));
	MAKE_CONST_MOCK14(Invoke, Return(Params...));
};

template <typename Return, typename... Params>
	requires (15u == sizeof...(Params))
class InvocableMockBase<Return, Params...>
{
public:
	MAKE_MOCK15(Invoke, Return(Params...));
	MAKE_CONST_MOCK15(Invoke, Return(Params...));
};

template <typename Return, typename... Params>
class InvocableMock
	: public InvocableMockBase<Return, Params...>
{
public:
	using ReturnT = Return;
	using ParamListT = std::tuple<Params...>;
	using InvocableMockBase<Return, Params...>::Invoke;

	constexpr ReturnT operator ()(Params... params)
	{
		return Invoke(std::forward<Params>(params)...);
	}

	constexpr ReturnT operator ()(Params... params) const
	{
		return Invoke(std::forward<Params>(params)...);
	}
};
