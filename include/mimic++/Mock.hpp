// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_MOCK_HPP
#define MIMICPP_MOCK_HPP

#pragma once

#include "mimic++/Expectation.hpp"
#include "mimic++/ExpectationPolicies/CallProperties.hpp"

#include <atomic>

namespace mimicpp::detail
{
	template <typename Signature>
	class MockBase
	{
	public:
		using SignatureT = Signature;

		~MockBase() = default;

		MockBase() = default;

		MockBase(const MockBase&) = delete;
		MockBase& operator =(const MockBase&) = delete;
		MockBase(MockBase&&) = default;
		MockBase& operator =(MockBase&&) = default;

	private:
		std::shared_ptr<ExpectationCollection<SignatureT>> m_Expectations{};
	};

	class UuidOwner
	{
	public:
		~UuidOwner() = default;

		[[nodiscard]]
		UuidOwner() = default;

		UuidOwner(const UuidOwner&) = delete;
		UuidOwner& operator =(const UuidOwner&) = delete;

		[[nodiscard]]
		UuidOwner(UuidOwner&& other) noexcept
			: m_Uuid{std::exchange(other.m_Uuid, Uuid{m_NextUuid++})}
		{
		}

		constexpr UuidOwner& operator =(UuidOwner&& other) noexcept
		{
			std::ranges::swap(m_Uuid, other.m_Uuid);
			return *this;
		}

		[[nodiscard]]
		constexpr Uuid uuid() const noexcept
		{
			return m_Uuid;
		}

	private:
		inline static std::atomic_size_t m_NextUuid{1u};
		Uuid m_Uuid{m_NextUuid++};
	};

	template <typename Signature, typename Builder, std::size_t... indices, typename... Args>
	[[nodiscard]]
	auto extend_builder_with_arg_policies(
		Builder&& builder,
		const std::index_sequence<indices...>,
		Args&&... args
	)
	{
		return (
			std::forward<Builder>(builder)
			| ...
			| expectation_policies::make_argument_matcher<Signature, indices>(
				std::bind_front(std::equal_to{}, std::forward<Args>(args))));
	}
}

namespace mimicpp
{
	template <typename T>
	class Mock;

	template <typename Return, typename... Params>
	class Mock<Return(Params...)>
	{
	public:
		using SignatureT = Return(Params...);
		using CallInfoT = call::Info<SignatureT>;

		~Mock() = default;

		Mock() = default;

		Mock(const Mock&) = delete;
		Mock& operator =(const Mock&) = delete;
		Mock(Mock&&) = default;
		Mock& operator =(Mock&&) = default;

		constexpr Return operator ()(Params... params) &
		{
			const CallInfoT call{
				.params = {std::ref(params)...},
				.fromUuid = m_Uuid.uuid(),
				.fromCategory = ValueCategory::lvalue,
				.fromConst = false
			};

			return m_Expectations->handle_call(call);
		}

		template <typename... Args>
		[[nodiscard]]
		constexpr auto expect(Args&&... args) &
		{
			return detail::extend_builder_with_arg_policies<SignatureT>(
				BasicExpectationBuilder<SignatureT, DummyFinalizePolicy>{
					m_Expectations,
					DummyFinalizePolicy{},
					std::tuple{}
				}
				| expectation_policies::Category<SignatureT, ValueCategory::lvalue>{}
				| expectation_policies::Constness<SignatureT, false>{},
				std::make_index_sequence<sizeof...(Args)>{},
				std::forward<Args>(args)...);
		}

	private:
		detail::UuidOwner m_Uuid{};
		std::shared_ptr<ExpectationCollection<SignatureT>> m_Expectations{
			std::make_shared<ExpectationCollection<SignatureT>>()
		};
	};
}

#endif
