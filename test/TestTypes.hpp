// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "mimic++/Call.hpp"
#include "mimic++/Expectation.hpp"

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
	using CallInfoT = mimicpp::call::Info<Signature>;
	using SubMatchT = mimicpp::call::SubMatchResultT;

	[[nodiscard]]
	static constexpr bool is_satisfied() noexcept
	{
		return false;
	}

	[[nodiscard]]
	static constexpr bool is_saturated() noexcept
	{
		return false;
	}

	[[nodiscard]]
	static constexpr SubMatchT matches(const CallInfoT& call) noexcept
	{
		return matches(call);
	}

	static constexpr void consume(const CallInfoT& call) noexcept
	{
	}
};

template <typename Signature, typename Policy, typename Projection>
// enable, when trompeloeil fully supports movable mocks
//requires mimicpp::expectation_policy_for<
//	std::remove_cvref_t<std::invoke_result_t<Projection, Policy>>,
//	Signature>
class PolicyFacade
{
public:
	using CallT = mimicpp::call::Info<Signature>;
	using SubMatchT = mimicpp::call::SubMatchResultT;

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
	using CallInfoT = mimicpp::call::Info<Signature>;
	using ReturnT = mimicpp::signature_return_type_t<Signature>;

	class Exception
	{
	};

	static ReturnT finalize_call(const CallInfoT& call)
	{
		throw Exception{};
	}
};

template <typename Signature, typename Policy, typename Projection>
// enable, when trompeloeil fully supports movable mocks
//requires mimicpp::finalize_policy_for<
//	std::remove_cvref_t<std::invoke_result_t<Projection, Policy>>,
//	Signature>
class FinalizerFacade
{
public:
	using CallInfoT = mimicpp::call::Info<Signature>;
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
