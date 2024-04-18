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
	constexpr auto extend_builder_with_arg_policies(
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

	template <typename Signature, typename... Args>
	constexpr auto make_expectation_builder(
		std::shared_ptr<ExpectationCollection<Signature>> expectations,
		Args&&... args
	)
	{
		return detail::extend_builder_with_arg_policies<Signature>(
			BasicExpectationBuilder<Signature, InitTimesPolicy, InitFinalizePolicy>{
				std::move(expectations),
				InitTimesPolicy{},
				InitFinalizePolicy{},
				std::tuple{}
			},
			std::make_index_sequence<sizeof...(Args)>{},
			std::forward<Args>(args)...);
	}

	template <typename Signature, typename Return, typename... Params>
	class MockBase
	{
	public:
		using SignatureT = Signature;
		using CallInfoT = call::Info<SignatureT>;

		MockBase(const MockBase&) = delete;
		MockBase& operator =(const MockBase&) = delete;

		template <typename... Args>
		[[nodiscard]]
		constexpr auto expect_call(Args&&... args) const &
		{
			return detail::make_expectation_builder(
						m_Expectations,
						std::forward<Args>(args)...)
					| expectation_policies::Category<ValueCategory::lvalue>{}
					| expectation_policies::Constness<true>{};
		}

		template <typename... Args>
		[[nodiscard]]
		constexpr auto expect_call(Args&&... args) &
		{
			return detail::make_expectation_builder(
						m_Expectations,
						std::forward<Args>(args)...)
					| expectation_policies::Category<ValueCategory::lvalue>{}
					| expectation_policies::Constness<false>{};
		}

		template <typename... Args>
		[[nodiscard]]
		constexpr auto expect_call(Args&&... args) const &&
		{
			return detail::make_expectation_builder(
						m_Expectations,
						std::forward<Args>(args)...)
					| expectation_policies::Category<ValueCategory::rvalue>{}
					| expectation_policies::Constness<true>{};
		}

		template <typename... Args>
		[[nodiscard]]
		constexpr auto expect_call(Args&&... args) &&
		{
			return detail::make_expectation_builder(
						m_Expectations,
						std::forward<Args>(args)...)
					| expectation_policies::Category<ValueCategory::rvalue>{}
					| expectation_policies::Constness<false>{};
		}

		template <typename... Args>
		[[nodiscard]]
		constexpr auto expect_lvalue_call(Args&&... args) const
		{
			return detail::make_expectation_builder(
						m_Expectations,
						std::forward<Args>(args)...)
					| expectation_policies::Category<ValueCategory::lvalue>{};
		}

		template <typename... Args>
		[[nodiscard]]
		constexpr auto expect_rvalue_call(Args&&... args) const
		{
			return detail::make_expectation_builder(
						m_Expectations,
						std::forward<Args>(args)...)
					| expectation_policies::Category<ValueCategory::rvalue>{};
		}

		template <typename... Args>
		[[nodiscard]]
		constexpr auto expect_const_call(Args&&... args) const
		{
			return detail::make_expectation_builder(
						m_Expectations,
						std::forward<Args>(args)...)
					| expectation_policies::Constness<true>{};
		}

		template <typename... Args>
		[[nodiscard]]
		constexpr auto expect_mutable_call(Args&&... args) const
		{
			return detail::make_expectation_builder(
						m_Expectations,
						std::forward<Args>(args)...)
					| expectation_policies::Constness<false>{};
		}

		template <typename... Args>
		[[nodiscard]]
		constexpr auto expect_any_call(Args&&... args) const
		{
			return detail::make_expectation_builder(
				m_Expectations,
				std::forward<Args>(args)...);
		}

	protected:
		~MockBase() = default;
		MockBase() = default;
		MockBase(MockBase&&) = default;
		MockBase& operator =(MockBase&&) = default;

		[[nodiscard]]
		constexpr Return handle_call(Params... params) const &
		{
			return m_Expectations->handle_call(
				CallInfoT{
					.params = {std::ref(params)...},
					.fromUuid = m_Uuid.uuid(),
					.fromCategory = ValueCategory::lvalue,
					.fromConst = true
				});
		}

		[[nodiscard]]
		constexpr Return handle_call(Params... params) &
		{
			return m_Expectations->handle_call(
				CallInfoT{
					.params = {std::ref(params)...},
					.fromUuid = m_Uuid.uuid(),
					.fromCategory = ValueCategory::lvalue,
					.fromConst = false
				});
		}

		[[nodiscard]]
		constexpr Return handle_call(Params... params) const &&
		{
			return m_Expectations->handle_call(
				CallInfoT{
					.params = {std::ref(params)...},
					.fromUuid = m_Uuid.uuid(),
					.fromCategory = ValueCategory::rvalue,
					.fromConst = true
				});
		}

		[[nodiscard]]
		constexpr Return handle_call(Params... params) &&
		{
			return m_Expectations->handle_call(
				CallInfoT{
					.params = {std::ref(params)...},
					.fromUuid = m_Uuid.uuid(),
					.fromCategory = ValueCategory::rvalue,
					.fromConst = false
				});
		}

	private:
		UuidOwner m_Uuid{};
		std::shared_ptr<ExpectationCollection<SignatureT>> m_Expectations{
			std::make_shared<ExpectationCollection<SignatureT>>()
		};
	};
}

namespace mimicpp
{
	template <typename T>
	class Mock;

	template <typename Return, typename... Params>
	class Mock<Return(Params...)>
		: public detail::MockBase<Return(Params...), Return, Params...>
	{
		using SuperT = detail::MockBase<Return(Params...), Return, Params...>;
		using SuperT::handle_call;

	public:
		using SignatureT = Return(Params...);
		using CallInfoT = call::Info<SignatureT>;
		using SuperT::expect_call;

		~Mock() = default;
		Mock() = default;

		Mock(const Mock&) = delete;
		Mock& operator =(const Mock&) = delete;
		Mock(Mock&&) = default;
		Mock& operator =(Mock&&) = default;

		constexpr Return operator ()(Params... params) const &
		{
			return handle_call(std::forward<Params>(params)...);
		}

		constexpr Return operator ()(Params... params) &
		{
			return handle_call(std::forward<Params>(params)...);
		}

		constexpr Return operator ()(Params... params) const &&
		{
			return std::move(*this).handle_call(std::forward<Params>(params)...);
		}

		constexpr Return operator ()(Params... params) &&
		{
			return std::move(*this).handle_call(std::forward<Params>(params)...);
		}
	};
}

#endif
