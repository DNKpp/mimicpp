// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_MATCHER_HPP
#define MIMICPP_MATCHER_HPP

#pragma once

#include "mimic++/Printer.hpp"
#include "mimic++/Utility.hpp"

#include <algorithm>
#include <concepts>
#include <functional>
#include <ranges>
#include <tuple>
#include <type_traits>

namespace mimicpp
{
	template <typename T, typename Target>
	concept matcher_for = std::same_as<T, std::remove_cvref_t<T>>
						&& std::is_move_constructible_v<T>
						&& std::destructible<T>
						&& requires(const T& matcher, Target& target)
						{
							{ matcher.matches(target) } -> std::convertible_to<bool>;
							{ matcher.describe(target) } -> std::convertible_to<StringT>;
						};
}

namespace mimicpp::matcher
{
	namespace detail
	{
		template <template <typename...> typename Trait, typename Return, typename Predicate, typename Target, typename OtherArgsTuple>
		struct is_applicable_helper;

		template <template <typename...> typename Trait, typename Return, typename Predicate, typename Target, typename... OtherArgs>
		struct is_applicable_helper<Trait, Return, Predicate, Target, std::tuple<OtherArgs...>>
			: public std::bool_constant<Trait<Return, Predicate, Target&, const OtherArgs&...>::value>
		{
		};

		template <typename Predicate, typename Target, typename OtherArgsTuple>
		concept applicable_predicate =
			is_applicable_helper<
				std::is_invocable_r,
				bool,
				Predicate,
				Target,
				OtherArgsTuple
			>::value;

		template <typename Predicate, typename Target, typename OtherArgsTuple>
		concept nothrow_applicable_predicate =
			applicable_predicate<Predicate, Target, OtherArgsTuple>
			&& is_applicable_helper<
				std::is_nothrow_invocable_r,
				bool,
				Predicate,
				Target,
				OtherArgsTuple
			>::value;
	}

	template <
		typename Predicate,
		typename AdditionalArgsTuple,
		template <typename Derived> typename... Policies
	>
		requires std::is_move_constructible_v<Predicate>
				&& std::is_move_constructible_v<AdditionalArgsTuple>
				&& (0 <= std::tuple_size_v<AdditionalArgsTuple>)
	class PredicateMatcher
		: public Policies<PredicateMatcher<Predicate, AdditionalArgsTuple, Policies...>>...
	{
	public:
		template <typename OtherPredicate>
		using RebindT = PredicateMatcher<OtherPredicate, AdditionalArgsTuple, Policies...>;

		[[nodiscard]]
		explicit constexpr PredicateMatcher(
			Predicate predicate,
			StringT fmt,
			AdditionalArgsTuple additionalArgs = AdditionalArgsTuple{}
		) noexcept(std::is_nothrow_move_constructible_v<Predicate>
					&& std::is_nothrow_move_constructible_v<AdditionalArgsTuple>)
			: m_Predicate{std::move(predicate)},
			m_FormatString{std::move(fmt)},
			m_AdditionalArgs{std::move(additionalArgs)}
		{
		}

		template <typename T>
			requires detail::applicable_predicate<const Predicate&, T, AdditionalArgsTuple>
		[[nodiscard]]
		constexpr bool matches(
			T& target
		) const noexcept(detail::nothrow_applicable_predicate<const Predicate&, T, AdditionalArgsTuple>)
		{
			return std::apply(
				[&, this](auto&... additionalArgs)
				{
					return std::invoke(
						m_Predicate,
						target,
						additionalArgs...);
				},
				m_AdditionalArgs);
		}

		template <typename T>
		[[nodiscard]]
		constexpr StringT describe(T& target) const
		{
			return std::apply(
				[&, this](auto&... additionalArgs)
				{
					// std::make_format_args requires lvalue-refs, so let's transform rvalue-refs to const lvalue-refs
					constexpr auto makeLvalue = [](auto&& val) noexcept -> const auto& { return val; };
					return format::vformat(
						m_FormatString,
						format::make_format_args(
							makeLvalue(mimicpp::print(target)),
							makeLvalue(mimicpp::print(additionalArgs))...));
				},
				m_AdditionalArgs);
		}

		[[nodiscard]]
		constexpr Predicate&& predicate() && noexcept
		{
			return std::move(m_Predicate);
		}

		[[nodiscard]]
		constexpr StringT&& format_string() && noexcept
		{
			return std::move(m_FormatString);
		}

		[[nodiscard]]
		constexpr AdditionalArgsTuple&& additional_args() && noexcept
		{
			return std::move(m_AdditionalArgs);
		}

	private:
		[[no_unique_address]] Predicate m_Predicate;
		StringT m_FormatString;
		AdditionalArgsTuple m_AdditionalArgs{};
	};

	template <
		template <typename> typename... Policies,
		typename Predicate,
		typename AdditionalArgsTuple = std::tuple<>>
	[[nodiscard]]
	constexpr PredicateMatcher<Predicate, AdditionalArgsTuple, Policies...> make_predicate_matcher(
		Predicate predicate,
		StringT formatString,
		AdditionalArgsTuple additionalArgsTuple = AdditionalArgsTuple{}
	) noexcept(std::is_nothrow_move_constructible_v<Predicate>
				&& std::is_nothrow_move_constructible_v<AdditionalArgsTuple>)
	{
		return PredicateMatcher<
			Predicate,
			AdditionalArgsTuple,
			Policies...>{
			std::move(predicate),
			std::move(formatString),
			std::move(additionalArgsTuple)
		};
	}

	template <typename Derived>
	class InvertiblePolicy
	{
	public:
		[[nodiscard]]
		constexpr auto operator !() &&
		{
			auto& self = static_cast<Derived&>(*this);

			auto notPredicate = std::not_fn(std::move(self).predicate());
			using InvertedMatcherT = typename Derived::template RebindT<decltype(notPredicate)>;
			return InvertedMatcherT{
				std::move(notPredicate),
				"!(" + std::move(self).format_string() + ")",
				std::move(self).additional_args()
			};
		}
	};
}

namespace mimicpp::matches
{
	inline static const matcher::PredicateMatcher _{
		AlwaysTruePredicate{},
		"{} without constraints",
		std::tuple{}
	};

	template <typename T>
	[[nodiscard]]
	constexpr auto eq(T&& value)
	{
		return matcher::make_predicate_matcher<matcher::InvertiblePolicy>(
			std::ranges::equal_to{},
			"{} == {}",
			std::tuple{std::forward<T>(value)});
	}

	template <typename T>
	[[nodiscard]]
	constexpr auto ne(T&& value)
	{
		return matcher::make_predicate_matcher<matcher::InvertiblePolicy>(
			std::ranges::not_equal_to{},
			"{} != {}",
			std::tuple{std::forward<T>(value)});
	}

	template <typename T>
	[[nodiscard]]
	constexpr auto lt(T&& value)
	{
		return matcher::make_predicate_matcher<matcher::InvertiblePolicy>(
			std::ranges::less{},
			"{} < {}",
			std::tuple{std::forward<T>(value)});
	}

	template <typename T>
	[[nodiscard]]
	constexpr auto le(T&& value)
	{
		return matcher::make_predicate_matcher<matcher::InvertiblePolicy>(
			std::ranges::less_equal{},
			"{} <= {}",
			std::tuple{std::forward<T>(value)});
	}

	template <typename T>
	[[nodiscard]]
	constexpr auto gt(T&& value)
	{
		return matcher::make_predicate_matcher<matcher::InvertiblePolicy>(
			std::ranges::greater{},
			"{} > {}",
			std::tuple{std::forward<T>(value)});
	}

	template <typename T>
	[[nodiscard]]
	constexpr auto ge(T&& value)
	{
		return matcher::make_predicate_matcher<matcher::InvertiblePolicy>(
			std::ranges::greater_equal{},
			"{} >= {}",
			std::tuple{std::forward<T>(value)});
	}

	template <typename UnaryPredicate>
	[[nodiscard]]
	constexpr auto predicate(UnaryPredicate&& predicate)
	{
		return matcher::make_predicate_matcher<matcher::InvertiblePolicy>(
			std::forward<UnaryPredicate>(predicate),
			"{} satisfies predicate");
	}
}

namespace mimicpp::matches::range
{
	template <std::ranges::forward_range Range, typename Comparator = std::equal_to<>>
	[[nodiscard]]
	constexpr auto eq(Range&& expected, Comparator comparator = Comparator{})
	{
		return matcher::make_predicate_matcher<matcher::InvertiblePolicy>(
			[comp = std::move(comparator)]<typename Target>(Target&& target, auto& range)
				requires std::predicate<
					const Comparator&,
					std::ranges::range_reference_t<Target>,
					std::ranges::range_reference_t<Range>>
			{
				return std::ranges::equal(
					target,
					range,
					std::ref(comp));
			},

			"{} range is equal to {}",
			std::tuple{std::views::all(std::forward<Range>(expected))});
	}
}

#endif
