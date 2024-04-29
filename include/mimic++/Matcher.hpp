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
	/**
	 * \defgroup EXPECTATION_MATCHER matchers
	 * \ingroup EXPECTATION_REQUIREMENT
	 * \brief Matchers check various argument properties.
	 * \details Matchers can be used to check various argument properties and are highly customizable. In general,
	 * they simply compare their arguments with a pre-defined predicate, but also provide a meaningful description.
	 *
	 * Most of the built-in matchers support the inversion operator (!), which then tests for the opposite condition.
	 *
	 * \attention Matchers receive their arguments as possibly non-const, which is due to workaround some restrictions
	 * on const qualified views. Either way, matchers should never modify any of their arguments.
	 *
	 *\{
	 */

	/**
	 * \brief The wildcard matcher, always matching.
	 */
	inline static const matcher::PredicateMatcher _{
		AlwaysTruePredicate{},
		"{} without constraints",
		std::tuple{}
	};

	/**
	 * \brief Tests, whether the target compares equal to the expected value.
	 * \tparam T Expected type.
	 * \param value Expected value.
	 */
	template <typename T>
	[[nodiscard]]
	constexpr auto eq(T&& value)
	{
		return matcher::make_predicate_matcher<matcher::InvertiblePolicy>(
			std::ranges::equal_to{},
			"{} == {}",
			std::tuple{std::forward<T>(value)});
	}

	/**
	 * \brief Tests, whether the target compares not equal to the expected value.
	 * \tparam T Expected type.
	 * \param value Expected value.
	 */
	template <typename T>
	[[nodiscard]]
	constexpr auto ne(T&& value)
	{
		return matcher::make_predicate_matcher<matcher::InvertiblePolicy>(
			std::ranges::not_equal_to{},
			"{} != {}",
			std::tuple{std::forward<T>(value)});
	}

	/**
	 * \brief Tests, whether the target is less than the expected value.
	 * \tparam T Expected type.
	 * \param value Expected value.
	 */
	template <typename T>
	[[nodiscard]]
	constexpr auto lt(T&& value)
	{
		return matcher::make_predicate_matcher<matcher::InvertiblePolicy>(
			std::ranges::less{},
			"{} < {}",
			std::tuple{std::forward<T>(value)});
	}

	/**
	 * \brief Tests, whether the target is less than or equal to the expected value.
	 * \tparam T Expected type.
	 * \param value Expected value.
	 */
	template <typename T>
	[[nodiscard]]
	constexpr auto le(T&& value)
	{
		return matcher::make_predicate_matcher<matcher::InvertiblePolicy>(
			std::ranges::less_equal{},
			"{} <= {}",
			std::tuple{std::forward<T>(value)});
	}

	/**
	 * \brief Tests, whether the target is greater than the expected value.
	 * \tparam T Expected type.
	 * \param value Expected value.
	 */
	template <typename T>
	[[nodiscard]]
	constexpr auto gt(T&& value)
	{
		return matcher::make_predicate_matcher<matcher::InvertiblePolicy>(
			std::ranges::greater{},
			"{} > {}",
			std::tuple{std::forward<T>(value)});
	}

	/**
	 * \brief Tests, whether the target is greater than or equal to the expected value.
	 * \tparam T Expected type.
	 * \param value Expected value.
	 */
	template <typename T>
	[[nodiscard]]
	constexpr auto ge(T&& value)
	{
		return matcher::make_predicate_matcher<matcher::InvertiblePolicy>(
			std::ranges::greater_equal{},
			"{} >= {}",
			std::tuple{std::forward<T>(value)});
	}

	/**
	 * \brief Tests, whether the target fulfills the given predicate.
	 * \tparam UnaryPredicate Predicate type.
	 * \param predicate The predicate to test.
	 * \param description The formatting string. May contain a ``{}``-token for the target.
	 */
	template <typename UnaryPredicate>
	[[nodiscard]]
	constexpr auto predicate(UnaryPredicate&& predicate, StringT description = "{} satisfies predicate")
	{
		return matcher::make_predicate_matcher<matcher::InvertiblePolicy>(
			std::forward<UnaryPredicate>(predicate),
			std::move(description));
	}

	/**
	 * \}
	 */
}

namespace mimicpp::matches::str
{
	/**
	 * \defgroup EXPECTATION_MATCHERS_STRING string matchers
	 * \ingroup EXPECTATION_REQUIREMENT
	 * \ingroup EXPECTATION_MATCHERS
	 * \brief String specific matchers.
	 *
	 *\{
	 */

	/**
	 * \brief Tests, whether the target string compares equal to the expected string.
	 * \tparam Char The character type.
	 * \tparam Traits The character traits type.
	 * \tparam Allocator The allocator type.
	 * \param expected The expected string.
	 */
	template <typename Char, typename Traits, typename Allocator>
	[[nodiscard]]
	constexpr auto eq(std::basic_string<Char, Traits, Allocator> expected)
	{
		using ViewT = std::basic_string_view<Char>;
		return matcher::make_predicate_matcher<matcher::InvertiblePolicy>(
			[](const ViewT target, const ViewT exp)
			{
				return target == exp;
			},
			"string {} is equal to {}",
			std::tuple{std::move(expected)});
	}

	/**
	 * \brief Tests, whether the target string compares equal to the expected string.
	 * \tparam Char The character type.
	 * \param expected The expected string.
	 */
	template <typename Char>
	[[nodiscard]]
	constexpr auto eq(const Char* expected)
	{
		return eq(
			std::basic_string<Char>{expected});
	}

	/**
	 * \}
	 */
}

namespace mimicpp::matches::range
{
	/**
	 * \defgroup EXPECTATION_MATCHER_RANGE range matchers
	 * \ingroup EXPECTATION_REQUIREMENT
	 * \ingroup EXPECTATION_MATCHER
	 * \brief Range specific matchers.
	 *
	 *\{
	 */

	/**
	 * \brief Tests, whether the target range compares equal to the expected range, by comparing them element-wise.
	 * \tparam Range Expected range type.
	 * \tparam Comparator Comparator type.
	 * \param expected The expected range.
	 * \param comparator The comparator.
	 */
	template <std::ranges::forward_range Range, typename Comparator = std::equal_to<>>
	[[nodiscard]]
	constexpr auto eq(Range&& expected, Comparator comparator = Comparator{})
	{
		return matcher::make_predicate_matcher<matcher::InvertiblePolicy>(
			[comp = std::move(comparator)]<typename Target>(Target&& target, auto& range)  // NOLINT(cppcoreguidelines-missing-std-forward)
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
			"range {} is equal to {}",
			std::tuple{std::views::all(std::forward<Range>(expected))});
	}

	/**
	 * \brief Tests, whether the target range is a permutation of the expected range, by comparing them element-wise.
	 * \tparam Range Expected range type.
	 * \tparam Comparator Comparator type.
	 * \param expected The expected range.
	 * \param comparator The comparator.
	 */
	template <std::ranges::forward_range Range, typename Comparator = std::equal_to<>>
	[[nodiscard]]
	constexpr auto unordered_eq(Range&& expected, Comparator comparator = Comparator{})
	{
		return matcher::make_predicate_matcher<matcher::InvertiblePolicy>(
			[comp = std::move(comparator)]<typename Target>(Target&& target, auto& range)  // NOLINT(cppcoreguidelines-missing-std-forward)
				requires std::predicate<
					const Comparator&,
					std::ranges::range_reference_t<Target>,
					std::ranges::range_reference_t<Range>>
			{
				return std::ranges::is_permutation(
					target,
					range,
					std::ref(comp));
			},
			"range {} is permutation of {}",
			std::tuple{std::views::all(std::forward<Range>(expected))});
	}

	/**
	 * \brief Tests, whether the target range is sorted, by applying the relation on each adjacent elements.
	 * \tparam Relation Relation type.
	 * \param relation The relation.
	 */
	template <typename Relation = std::ranges::less>
	[[nodiscard]]
	constexpr auto is_sorted(Relation relation = Relation{})
	{
		return matches::predicate(
			[rel = std::move(relation)]<typename Target>(Target&& target)  // NOLINT(cppcoreguidelines-missing-std-forward)
				requires std::equivalence_relation<
					const Relation&,
					std::ranges::range_reference_t<Target>,
					std::ranges::range_reference_t<Target>>
			{
				return std::ranges::is_sorted(
					target,
					std::ref(rel));
			},
			"range {} is sorted");
	}

	/**
	 * \brief Tests, whether the target range is empty.
	 */
	[[nodiscard]]
	constexpr auto is_empty()
	{
		return matches::predicate(
			[](std::ranges::range auto&& target)
			{
				return std::ranges::empty(target);
			},
			"range {} is empty");
	}

	/**
	 * \brief Tests, whether the target range has the expected size.
	 * \param expected The expected size.
	 */
	[[nodiscard]]
	constexpr auto has_size(const std::integral auto expected)
	{
		return matcher::make_predicate_matcher<matcher::InvertiblePolicy>(
			[](std::ranges::range auto&& target, const std::integral auto size)
			{
				return std::cmp_equal(
					size,
					std::ranges::size(target));
			},
			"range {} has size {}",
			std::tuple{expected});
	}

	/**
	 * \}
	 */
}

#endif
