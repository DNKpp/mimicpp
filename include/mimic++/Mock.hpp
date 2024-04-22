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
	template <typename Signature>
	class MockBase;

	template <typename Return, typename... Params>
	class MockBase<Return(Params...)>
	{
	protected:
		using SignatureT = Return(Params...);
		static constexpr auto category{ValueCategory::any};
		static constexpr auto constness{Constness::non_const};

	public:
		MockBase(const MockBase&) = delete;
		MockBase& operator =(const MockBase&) = delete;

		template <typename... Args>
			requires (sizeof...(Params) == sizeof...(Args))
		[[nodiscard]]
		constexpr auto expect_call(Args&&... args)
		{
			return detail::make_expectation_builder(
						m_Expectations,
						std::forward<Args>(args)...)
					| expectation_policies::Category<category>{}
					| expectation_policies::Constness<constness>{};
		}

	protected:
		~MockBase() = default;

		[[nodiscard]]
		MockBase() = default;

		[[nodiscard]]
		MockBase(MockBase&&) = default;
		MockBase& operator =(MockBase&&) = default;

		[[nodiscard]]
		constexpr Return handle_call(const std::source_location& from, Params... params)
		{
			using CallInfoT = call::Info<Return, Params...>;

			return m_Expectations->handle_call(
				CallInfoT{
					.params = {std::ref(params)...},
					.fromCategory = category,
					.fromConstness = constness,
					.fromSourceLocation = from
				});
		}

	private:
		std::shared_ptr<ExpectationCollection<SignatureT>> m_Expectations{
			std::make_shared<ExpectationCollection<SignatureT>>()
		};
	};

	template <typename Return, typename... Params>
	class MockBase<Return(Params...) const>
	{
	protected:
		using SignatureT = Return(Params...) const;
		static constexpr auto category{ValueCategory::any};
		static constexpr auto constness{Constness::as_const};

	public:
		MockBase(const MockBase&) = delete;
		MockBase& operator =(const MockBase&) = delete;

		template <typename... Args>
			requires (sizeof...(Params) == sizeof...(Args))
		[[nodiscard]]
		constexpr auto expect_call(Args&&... args) const
		{
			return detail::make_expectation_builder(
						m_Expectations,
						std::forward<Args>(args)...)
					| expectation_policies::Category<category>{}
					| expectation_policies::Constness<constness>{};
		}

	protected:
		~MockBase() = default;

		[[nodiscard]]
		MockBase() = default;

		[[nodiscard]]
		MockBase(MockBase&&) = default;
		MockBase& operator =(MockBase&&) = default;

		[[nodiscard]]
		constexpr Return handle_call(const std::source_location& from, Params... params) const
		{
			using CallInfoT = call::Info<Return, Params...>;

			return m_Expectations->handle_call(
				CallInfoT{
					.params = {std::ref(params)...},
					.fromCategory = category,
					.fromConstness = constness,
					.fromSourceLocation = from
				});
		}

	private:
		std::shared_ptr<ExpectationCollection<SignatureT>> m_Expectations{
			std::make_shared<ExpectationCollection<SignatureT>>()
		};
	};

	template <typename Return, typename... Params>
	class MockBase<Return(Params...) &>
	{
	protected:
		using SignatureT = Return(Params...) &;
		static constexpr auto category{ValueCategory::lvalue};
		static constexpr auto constness{Constness::non_const};

	public:
		MockBase(const MockBase&) = delete;
		MockBase& operator =(const MockBase&) = delete;

		template <typename... Args>
			requires (sizeof...(Params) == sizeof...(Args))
		[[nodiscard]]
		constexpr auto expect_call(Args&&... args) &
		{
			return detail::make_expectation_builder(
						m_Expectations,
						std::forward<Args>(args)...)
					| expectation_policies::Category<category>{}
					| expectation_policies::Constness<constness>{};
		}

	protected:
		~MockBase() = default;

		[[nodiscard]]
		MockBase() = default;

		[[nodiscard]]
		MockBase(MockBase&&) = default;
		MockBase& operator =(MockBase&&) = default;

		[[nodiscard]]
		constexpr Return handle_call(const std::source_location& from, Params... params) &
		{
			using CallInfoT = call::Info<Return, Params...>;

			return m_Expectations->handle_call(
				CallInfoT{
					.params = {std::ref(params)...},
					.fromCategory = category,
					.fromConstness = constness,
					.fromSourceLocation = from
				});
		}

	private:
		std::shared_ptr<ExpectationCollection<SignatureT>> m_Expectations{
			std::make_shared<ExpectationCollection<SignatureT>>()
		};
	};

	template <typename Return, typename... Params>
	class MockBase<Return(Params...) const &>
	{
	protected:
		using SignatureT = Return(Params...) const &;
		static constexpr auto category{ValueCategory::lvalue};
		static constexpr auto constness{Constness::as_const};

	public:
		MockBase(const MockBase&) = delete;
		MockBase& operator =(const MockBase&) = delete;

		template <typename... Args>
			requires (sizeof...(Params) == sizeof...(Args))
		[[nodiscard]]
		constexpr auto expect_call(Args&&... args) const &
		{
			return detail::make_expectation_builder(
						m_Expectations,
						std::forward<Args>(args)...)
					| expectation_policies::Category<category>{}
					| expectation_policies::Constness<constness>{};
		}

	protected:
		~MockBase() = default;

		[[nodiscard]]
		MockBase() = default;

		[[nodiscard]]
		MockBase(MockBase&&) = default;
		MockBase& operator =(MockBase&&) = default;

		[[nodiscard]]
		constexpr Return handle_call(const std::source_location& from, Params... params) const &
		{
			using CallInfoT = call::Info<Return, Params...>;

			return m_Expectations->handle_call(
				CallInfoT{
					.params = {std::ref(params)...},
					.fromCategory = category,
					.fromConstness = constness,
					.fromSourceLocation = from
				});
		}

	private:
		std::shared_ptr<ExpectationCollection<SignatureT>> m_Expectations{
			std::make_shared<ExpectationCollection<SignatureT>>()
		};
	};

	template <typename Return, typename... Params>
	class MockBase<Return(Params...) &&>
	{
	protected:
		using SignatureT = Return(Params...) &&;
		static constexpr auto category{ValueCategory::rvalue};
		static constexpr auto constness{Constness::non_const};

	public:
		MockBase(const MockBase&) = delete;
		MockBase& operator =(const MockBase&) = delete;

		template <typename... Args>
			requires (sizeof...(Params) == sizeof...(Args))
		[[nodiscard]]
		constexpr auto expect_call(Args&&... args) &&
		{
			return detail::make_expectation_builder(
						m_Expectations,
						std::forward<Args>(args)...)
					| expectation_policies::Category<category>{}
					| expectation_policies::Constness<constness>{};
		}

	protected:
		~MockBase() = default;

		[[nodiscard]]
		MockBase() = default;

		[[nodiscard]]
		MockBase(MockBase&&) = default;
		MockBase& operator =(MockBase&&) = default;

		[[nodiscard]]
		constexpr Return handle_call(const std::source_location& from, Params... params) &&
		{
			using CallInfoT = call::Info<Return, Params...>;

			return m_Expectations->handle_call(
				CallInfoT{
					.params = {std::ref(params)...},
					.fromCategory = category,
					.fromConstness = constness,
					.fromSourceLocation = from
				});
		}

	private:
		std::shared_ptr<ExpectationCollection<SignatureT>> m_Expectations{
			std::make_shared<ExpectationCollection<SignatureT>>()
		};
	};

	template <typename Return, typename... Params>
	class MockBase<Return(Params...) const &&>
	{
	protected:
		using SignatureT = Return(Params...) const &&;
		static constexpr auto category{ValueCategory::rvalue};
		static constexpr auto constness{Constness::as_const};

	public:
		MockBase(const MockBase&) = delete;
		MockBase& operator =(const MockBase&) = delete;

		template <typename... Args>
			requires (sizeof...(Params) == sizeof...(Args))
		[[nodiscard]]
		constexpr auto expect_call(Args&&... args) const &&
		{
			return detail::make_expectation_builder(
						m_Expectations,
						std::forward<Args>(args)...)
					| expectation_policies::Category<category>{}
					| expectation_policies::Constness<constness>{};
		}

	protected:
		~MockBase() = default;

		[[nodiscard]]
		MockBase() = default;

		[[nodiscard]]
		MockBase(MockBase&&) = default;
		MockBase& operator =(MockBase&&) = default;

		[[nodiscard]]
		constexpr Return handle_call(const std::source_location& from, Params... params) const &&
		{
			using CallInfoT = call::Info<Return, Params...>;

			return m_Expectations->handle_call(
				CallInfoT{
					.params = {std::ref(params)...},
					.fromCategory = category,
					.fromConstness = constness,
					.fromSourceLocation = from
				});
		}

	private:
		std::shared_ptr<ExpectationCollection<SignatureT>> m_Expectations{
			std::make_shared<ExpectationCollection<SignatureT>>()
		};
	};
}

namespace mimicpp
{
	template <typename Signature>
	class Mock;

	template <typename Return, typename... Params>
	class Mock<Return(Params...)>
		: public detail::MockBase<Return(Params...)>
	{
		using SuperT = detail::MockBase<Return(Params...)>;

	public:
		~Mock() = default;

		[[nodiscard]]
		Mock() = default;

		Mock(const Mock&) = delete;
		Mock& operator =(const Mock&) = delete;

		[[nodiscard]]
		Mock(Mock&&) = default;
		Mock& operator =(Mock&&) = default;

		constexpr Return operator ()(Params... params, const std::source_location& from = std::source_location::current())
		{
			return SuperT::handle_call(
				from,
				std::forward<Params>(params)...);
		}
	};

	template <typename Return, typename... Params>
	class Mock<Return(Params...) const>
		: public detail::MockBase<Return(Params...) const>
	{
		using SuperT = detail::MockBase<Return(Params...) const>;

	public:
		~Mock() = default;

		[[nodiscard]]
		Mock() = default;

		Mock(const Mock&) = delete;
		Mock& operator =(const Mock&) = delete;

		[[nodiscard]]
		Mock(Mock&&) = default;
		Mock& operator =(Mock&&) = default;

		constexpr Return operator ()(Params... params, const std::source_location& from = std::source_location::current()) const
		{
			return SuperT::handle_call(
				from,
				std::forward<Params>(params)...);
		}
	};

	template <typename Return, typename... Params>
	class Mock<Return(Params...) &>
		: public detail::MockBase<Return(Params...) &>
	{
		using SuperT = detail::MockBase<Return(Params...) &>;

	public:
		~Mock() = default;

		[[nodiscard]]
		Mock() = default;

		Mock(const Mock&) = delete;
		Mock& operator =(const Mock&) = delete;

		[[nodiscard]]
		Mock(Mock&&) = default;
		Mock& operator =(Mock&&) = default;

		constexpr Return operator ()(Params... params, const std::source_location& from = std::source_location::current()) &
		{
			return SuperT::handle_call(
				from,
				std::forward<Params>(params)...);
		}
	};

	template <typename Return, typename... Params>
	class Mock<Return(Params...) const &>
		: public detail::MockBase<Return(Params...) const &>
	{
		using SuperT = detail::MockBase<Return(Params...) const &>;

	public:
		~Mock() = default;

		[[nodiscard]]
		Mock() = default;

		Mock(const Mock&) = delete;
		Mock& operator =(const Mock&) = delete;

		[[nodiscard]]
		Mock(Mock&&) = default;
		Mock& operator =(Mock&&) = default;

		constexpr Return operator ()(Params... params, const std::source_location& from = std::source_location::current()) const &
		{
			return SuperT::handle_call(
				from,
				std::forward<Params>(params)...);
		}
	};

	template <typename Return, typename... Params>
	class Mock<Return(Params...) &&>
		: public detail::MockBase<Return(Params...) &&>
	{
		using SuperT = detail::MockBase<Return(Params...) &&>;

	public:
		~Mock() = default;

		[[nodiscard]]
		Mock() = default;

		Mock(const Mock&) = delete;
		Mock& operator =(const Mock&) = delete;

		[[nodiscard]]
		Mock(Mock&&) = default;
		Mock& operator =(Mock&&) = default;

		constexpr Return operator ()(Params... params, const std::source_location& from = std::source_location::current()) &&
		{
			return std::move(*this).SuperT::handle_call(
				from,
				std::forward<Params>(params)...);
		}
	};

	template <typename Return, typename... Params>
	class Mock<Return(Params...) const &&>
		: public detail::MockBase<Return(Params...) const &&>
	{
		using SuperT = detail::MockBase<Return(Params...) const &&>;

	public:
		~Mock() = default;

		[[nodiscard]]
		Mock() = default;

		Mock(const Mock&) = delete;
		Mock& operator =(const Mock&) = delete;

		[[nodiscard]]
		Mock(Mock&&) = default;
		Mock& operator =(Mock&&) = default;

		constexpr Return operator ()(Params... params, const std::source_location& from = std::source_location::current()) const &&
		{
			return std::move(*this).handle_call(
				from,
				std::forward<Params>(params)...);
		}
	};

	template <typename Return, typename... Params>
	class Mock<Return(Params...) noexcept>
		: public detail::MockBase<Return(Params...)>
	{
		using SuperT = detail::MockBase<Return(Params...)>;

	public:
		~Mock() = default;

		[[nodiscard]]
		Mock() = default;

		Mock(const Mock&) = delete;
		Mock& operator =(const Mock&) = delete;

		[[nodiscard]]
		Mock(Mock&&) = default;
		Mock& operator =(Mock&&) = default;

		constexpr Return operator ()(Params... params, const std::source_location& from = std::source_location::current()) noexcept
		{
			return SuperT::handle_call(
				from,
				std::forward<Params>(params)...);
		}
	};

	template <typename Return, typename... Params>
	class Mock<Return(Params...) const noexcept>
		: public detail::MockBase<Return(Params...) const>
	{
		using SuperT = detail::MockBase<Return(Params...) const>;

	public:
		~Mock() = default;

		[[nodiscard]]
		Mock() = default;

		Mock(const Mock&) = delete;
		Mock& operator =(const Mock&) = delete;

		[[nodiscard]]
		Mock(Mock&&) = default;
		Mock& operator =(Mock&&) = default;

		constexpr Return operator ()(Params... params, const std::source_location& from = std::source_location::current()) const noexcept
		{
			return SuperT::handle_call(
				from,
				std::forward<Params>(params)...);
		}
	};

	template <typename Return, typename... Params>
	class Mock<Return(Params...) & noexcept>
		: public detail::MockBase<Return(Params...) &>
	{
		using SuperT = detail::MockBase<Return(Params...) &>;

	public:
		~Mock() = default;

		[[nodiscard]]
		Mock() = default;

		Mock(const Mock&) = delete;
		Mock& operator =(const Mock&) = delete;

		[[nodiscard]]
		Mock(Mock&&) = default;
		Mock& operator =(Mock&&) = default;

		constexpr Return operator ()(Params... params, const std::source_location& from = std::source_location::current()) & noexcept
		{
			return SuperT::handle_call(
				from,
				std::forward<Params>(params)...);
		}
	};

	template <typename Return, typename... Params>
	class Mock<Return(Params...) const & noexcept>
		: public detail::MockBase<Return(Params...) const &>
	{
		using SuperT = detail::MockBase<Return(Params...) const &>;

	public:
		~Mock() = default;

		[[nodiscard]]
		Mock() = default;

		Mock(const Mock&) = delete;
		Mock& operator =(const Mock&) = delete;

		[[nodiscard]]
		Mock(Mock&&) = default;
		Mock& operator =(Mock&&) = default;

		constexpr Return operator ()(Params... params, const std::source_location& from = std::source_location::current()) const & noexcept
		{
			return SuperT::handle_call(
				from,
				std::forward<Params>(params)...);
		}
	};

	template <typename Return, typename... Params>
	class Mock<Return(Params...) && noexcept>
		: public detail::MockBase<Return(Params...) &&>
	{
		using SuperT = detail::MockBase<Return(Params...) &&>;

	public:
		~Mock() = default;

		[[nodiscard]]
		Mock() = default;

		Mock(const Mock&) = delete;
		Mock& operator =(const Mock&) = delete;

		[[nodiscard]]
		Mock(Mock&&) = default;
		Mock& operator =(Mock&&) = default;

		constexpr Return operator ()(Params... params, const std::source_location& from = std::source_location::current()) && noexcept
		{
			return std::move(*this).SuperT::handle_call(
				from,
				std::forward<Params>(params)...);
		}
	};

	template <typename Return, typename... Params>
	class Mock<Return(Params...) const && noexcept>
		: public detail::MockBase<Return(Params...) const &&>
	{
		using SuperT = detail::MockBase<Return(Params...) const &&>;

	public:
		~Mock() = default;

		[[nodiscard]]
		Mock() = default;

		Mock(const Mock&) = delete;
		Mock& operator =(const Mock&) = delete;

		[[nodiscard]]
		Mock(Mock&&) = default;
		Mock& operator =(Mock&&) = default;

		constexpr Return operator ()(Params... params, const std::source_location& from = std::source_location::current()) const && noexcept
		{
			return std::move(*this).SuperT::handle_call(
				from,
				std::forward<Params>(params)...);
		}
	};
}

#endif
