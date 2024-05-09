// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_MOCK_HPP
#define MIMICPP_MOCK_HPP

#pragma once

#include "mimic++/Expectation.hpp"
#include "mimic++/ExpectationBuilder.hpp"
#include "mimic++/ExpectationPolicies.hpp"

namespace mimicpp::detail
{
	template <typename T, typename Target>
	concept requirement_for = std::equality_comparable_with<T, Target>
							|| matcher_for<std::remove_cvref_t<T>, Target>;

	template <ValueCategory category, Constness constness, typename Return, typename... Params>
	class BasicMockFrontend
	{
	protected:
		using ExpectationCollectionT = ExpectationCollection<Return(Params...)>;

		[[nodiscard]]
		explicit BasicMockFrontend(
			std::shared_ptr<ExpectationCollectionT> collection = std::make_shared<ExpectationCollectionT>()
		) noexcept
			: m_Expectations{std::move(collection)}
		{
		}

		constexpr Return handle_call(
			std::tuple<std::reference_wrapper<std::remove_reference_t<Params>>...> params,
			const std::source_location& from
		) const
		{
			using CallInfoT = call::Info<Return, Params...>;

			return m_Expectations->handle_call(
				CallInfoT{
					.args = std::move(params),
					.fromCategory = category,
					.fromConstness = constness,
					.fromSourceLocation = from
				});
		}

		template <typename... Args>
			requires (... && requirement_for<Args, Params>)
		[[nodiscard]]
		constexpr auto make_expectation_builder(Args&&... args) const
		{
			return detail::make_expectation_builder(
						m_Expectations,
						std::forward<Args>(args)...)
					| expectation_policies::Category<category>{}
					| expectation_policies::Constness<constness>{};
		}

	private:
		std::shared_ptr<ExpectationCollectionT> m_Expectations;
	};

	template <typename Signature>
	class MockFrontend;

	template <typename Return, typename... Params>
	class MockFrontend<Return(Params...)>
		: private BasicMockFrontend<
			ValueCategory::any,
			Constness::non_const,
			Return,
			Params...>
	{
		using SuperT = BasicMockFrontend<
			ValueCategory::any,
			Constness::non_const,
			Return,
			Params...>;

	public:
		using SignatureT = Return(Params...);

		constexpr Return operator ()(Params... params, const std::source_location& from = std::source_location::current())
		{
			return SuperT::handle_call(
				std::tuple{std::ref(params)...},
				from);
		}

		template <typename... Args>
			requires (... && requirement_for<Args, Params>)
		[[nodiscard]]
		constexpr auto expect_call(Args&&... args)
		{
			return SuperT::make_expectation_builder(
				std::forward<Args>(args)...);
		}
	};

	template <typename Return, typename... Params>
	class MockFrontend<Return(Params...) const>
		: public BasicMockFrontend<
			ValueCategory::any,
			Constness::as_const,
			Return,
			Params...>
	{
		using SuperT = BasicMockFrontend<
			ValueCategory::any,
			Constness::as_const,
			Return,
			Params...>;

	public:
		using SignatureT = Return(Params...) const;

		constexpr Return operator ()(Params... params, const std::source_location& from = std::source_location::current()) const
		{
			return SuperT::handle_call(
				std::tuple{std::ref(params)...},
				from);
		}

		template <typename... Args>
			requires (... && requirement_for<Args, Params>)
		[[nodiscard]]
		constexpr auto expect_call(Args&&... args) const
		{
			return SuperT::make_expectation_builder(
				std::forward<Args>(args)...);
		}
	};

	template <typename Return, typename... Params>
	class MockFrontend<Return(Params...) &>
		: private BasicMockFrontend<
			ValueCategory::lvalue,
			Constness::non_const,
			Return,
			Params...>
	{
		using SuperT = BasicMockFrontend<
			ValueCategory::lvalue,
			Constness::non_const,
			Return,
			Params...>;

	public:
		using SignatureT = Return(Params...) &;

		constexpr Return operator ()(Params... params, const std::source_location& from = std::source_location::current()) &
		{
			return SuperT::handle_call(
				std::tuple{std::ref(params)...},
				from);
		}

		template <typename... Args>
			requires (... && requirement_for<Args, Params>)
		[[nodiscard]]
		constexpr auto expect_call(Args&&... args) &
		{
			return SuperT::make_expectation_builder(
				std::forward<Args>(args)...);
		}
	};

	template <typename Return, typename... Params>
	class MockFrontend<Return(Params...) const &>
		: public BasicMockFrontend<
			ValueCategory::lvalue,
			Constness::as_const,
			Return,
			Params...>
	{
		using SuperT = BasicMockFrontend<
			ValueCategory::lvalue,
			Constness::as_const,
			Return,
			Params...>;

	public:
		using SignatureT = Return(Params...) const &;

		constexpr Return operator ()(Params... params, const std::source_location& from = std::source_location::current()) const &
		{
			return SuperT::handle_call(
				std::tuple{std::ref(params)...},
				from);
		}

		template <typename... Args>
			requires (... && requirement_for<Args, Params>)
		[[nodiscard]]
		constexpr auto expect_call(Args&&... args) const &
		{
			return SuperT::make_expectation_builder(
				std::forward<Args>(args)...);
		}
	};

	template <typename Return, typename... Params>
	class MockFrontend<Return(Params...) &&>
		: private BasicMockFrontend<
			ValueCategory::rvalue,
			Constness::non_const,
			Return,
			Params...>
	{
		using SuperT = BasicMockFrontend<
			ValueCategory::rvalue,
			Constness::non_const,
			Return,
			Params...>;

	public:
		using SignatureT = Return(Params...) &&;

		constexpr Return operator ()(Params... params, const std::source_location& from = std::source_location::current()) &&
		{
			return SuperT::handle_call(
				std::tuple{std::ref(params)...},
				from);
		}

		template <typename... Args>
			requires (... && requirement_for<Args, Params>)
		[[nodiscard]]
		constexpr auto expect_call(Args&&... args) &&
		{
			return SuperT::make_expectation_builder(
				std::forward<Args>(args)...);
		}
	};

	template <typename Return, typename... Params>
	class MockFrontend<Return(Params...) const &&>
		: public BasicMockFrontend<
			ValueCategory::rvalue,
			Constness::as_const,
			Return,
			Params...>
	{
		using SuperT = BasicMockFrontend<
			ValueCategory::rvalue,
			Constness::as_const,
			Return,
			Params...>;

	public:
		using SignatureT = Return(Params...) const &&;

		constexpr Return operator ()(Params... params, const std::source_location& from = std::source_location::current()) const &&
		{
			return SuperT::handle_call(
				std::tuple{std::ref(params)...},
				from);
		}

		template <typename... Args>
			requires (... && requirement_for<Args, Params>)
		[[nodiscard]]
		constexpr auto expect_call(Args&&... args) const &&
		{
			return SuperT::make_expectation_builder(
				std::forward<Args>(args)...);
		}
	};

	template <typename Return, typename... Params>
	class MockFrontend<Return(Params...) noexcept>
		: private BasicMockFrontend<
			ValueCategory::any,
			Constness::non_const,
			Return,
			Params...>
	{
		using SuperT = BasicMockFrontend<
			ValueCategory::any,
			Constness::non_const,
			Return,
			Params...>;

	public:
		using SignatureT = Return(Params...) noexcept;

		constexpr Return operator ()(Params... params, const std::source_location& from = std::source_location::current()) noexcept
		{
			return SuperT::handle_call(
				std::tuple{std::ref(params)...},
				from);
		}

		template <typename... Args>
			requires (... && requirement_for<Args, Params>)
		[[nodiscard]]
		constexpr auto expect_call(Args&&... args)
		{
			return SuperT::make_expectation_builder(
				std::forward<Args>(args)...);
		}
	};

	template <typename Return, typename... Params>
	class MockFrontend<Return(Params...) const noexcept>
		: public BasicMockFrontend<
			ValueCategory::any,
			Constness::as_const,
			Return,
			Params...>
	{
		using SuperT = BasicMockFrontend<
			ValueCategory::any,
			Constness::as_const,
			Return,
			Params...>;

	public:
		using SignatureT = Return(Params...) const noexcept;

		constexpr Return operator ()(
			Params... params,
			const std::source_location& from = std::source_location::current()
		) const noexcept
		{
			return SuperT::handle_call(
				std::tuple{std::ref(params)...},
				from);
		}

		template <typename... Args>
			requires (... && requirement_for<Args, Params>)
		[[nodiscard]]
		constexpr auto expect_call(Args&&... args) const
		{
			return SuperT::make_expectation_builder(
				std::forward<Args>(args)...);
		}
	};

	template <typename Return, typename... Params>
	class MockFrontend<Return(Params...) & noexcept>
		: private BasicMockFrontend<
			ValueCategory::lvalue,
			Constness::non_const,
			Return,
			Params...>
	{
		using SuperT = BasicMockFrontend<
			ValueCategory::lvalue,
			Constness::non_const,
			Return,
			Params...>;

	public:
		using SignatureT = Return(Params...) & noexcept;

		constexpr Return operator ()(Params... params, const std::source_location& from = std::source_location::current()) & noexcept
		{
			return SuperT::handle_call(
				std::tuple{std::ref(params)...},
				from);
		}

		template <typename... Args>
			requires (... && requirement_for<Args, Params>)
		[[nodiscard]]
		constexpr auto expect_call(Args&&... args) &
		{
			return SuperT::make_expectation_builder(
				std::forward<Args>(args)...);
		}
	};

	template <typename Return, typename... Params>
	class MockFrontend<Return(Params...) const & noexcept>
		: public BasicMockFrontend<
			ValueCategory::lvalue,
			Constness::as_const,
			Return,
			Params...>
	{
		using SuperT = BasicMockFrontend<
			ValueCategory::lvalue,
			Constness::as_const,
			Return,
			Params...>;

	public:
		using SignatureT = Return(Params...) const & noexcept;

		constexpr Return operator ()(
			Params... params,
			const std::source_location& from = std::source_location::current()
		) const & noexcept
		{
			return SuperT::handle_call(
				std::tuple{std::ref(params)...},
				from);
		}

		template <typename... Args>
			requires (... && requirement_for<Args, Params>)
		[[nodiscard]]
		constexpr auto expect_call(Args&&... args) const &
		{
			return SuperT::make_expectation_builder(
				std::forward<Args>(args)...);
		}
	};

	template <typename Return, typename... Params>
	class MockFrontend<Return(Params...) && noexcept>
		: private BasicMockFrontend<
			ValueCategory::rvalue,
			Constness::non_const,
			Return,
			Params...>
	{
		using SuperT = BasicMockFrontend<
			ValueCategory::rvalue,
			Constness::non_const,
			Return,
			Params...>;

	public:
		using SignatureT = Return(Params...) && noexcept;

		constexpr Return operator ()(Params... params, const std::source_location& from = std::source_location::current()) && noexcept
		{
			return SuperT::handle_call(
				std::tuple{std::ref(params)...},
				from);
		}

		template <typename... Args>
			requires (... && requirement_for<Args, Params>)
		[[nodiscard]]
		constexpr auto expect_call(Args&&... args) &&
		{
			return SuperT::make_expectation_builder(
				std::forward<Args>(args)...);
		}
	};

	template <typename Return, typename... Params>
	class MockFrontend<Return(Params...) const && noexcept>
		: public BasicMockFrontend<
			ValueCategory::rvalue,
			Constness::as_const,
			Return,
			Params...>
	{
		using SuperT = BasicMockFrontend<
			ValueCategory::rvalue,
			Constness::as_const,
			Return,
			Params...>;

	public:
		using SignatureT = Return(Params...) const && noexcept;

		constexpr Return operator ()(
			Params... params,
			const std::source_location& from = std::source_location::current()
		) const && noexcept
		{
			return SuperT::handle_call(
				std::tuple{std::ref(params)...},
				from);
		}

		template <typename... Args>
			requires (... && requirement_for<Args, Params>)
		[[nodiscard]]
		constexpr auto expect_call(Args&&... args) const &&
		{
			return SuperT::make_expectation_builder(
				std::forward<Args>(args)...);
		}
	};
}

namespace mimicpp
{
	template <typename FirstSignature, typename... OtherSignatures>
		requires is_overload_set_v<FirstSignature, OtherSignatures...>
	class Mock
		: public detail::MockFrontend<FirstSignature>,
			public detail::MockFrontend<OtherSignatures>...
	{
	public:
		using detail::MockFrontend<FirstSignature>::operator();
		using detail::MockFrontend<FirstSignature>::expect_call;
		using detail::MockFrontend<OtherSignatures>::operator()...;
		using detail::MockFrontend<OtherSignatures>::expect_call...;

		~Mock() = default;

		[[nodiscard]]
		Mock() = default;

		Mock(const Mock&) = delete;
		Mock& operator =(const Mock&) = delete;

		[[nodiscard]]
		Mock(Mock&&) = default;
		Mock& operator =(Mock&&) = default;
	};
}

#endif
