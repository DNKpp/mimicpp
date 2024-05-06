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
#include <optional>
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
							{ matcher.describe() } -> std::convertible_to<std::optional<StringT>>;
						};

	/**
	 * \brief Generic matcher and the basic building block of most of the built-in matchers.
	 * \tparam Predicate The predicate type.
	 * \tparam AdditionalArgs Addition argument types.
	 * \ingroup EXPECTATION_REQUIREMENT
	 * \ingroup EXPECTATION_MATCHER
	 */
	template <typename Predicate, typename... AdditionalArgs>
		requires std::is_move_constructible_v<Predicate>
				&& (... && std::is_move_constructible_v<AdditionalArgs>)
	class PredicateMatcher
	{
	public:
		[[nodiscard]]
		explicit constexpr PredicateMatcher(
			Predicate predicate,
			StringT fmt,
			StringT invertedFmt,
			std::tuple<AdditionalArgs...> additionalArgs = std::tuple{}
		) noexcept(std::is_nothrow_move_constructible_v<Predicate>
					&& (... && std::is_nothrow_move_constructible_v<AdditionalArgs>))
			: m_Predicate{std::move(predicate)},
			m_FormatString{std::move(fmt)},
			m_InvertedFormatString{std::move(invertedFmt)},
			m_AdditionalArgs{std::move(additionalArgs)}
		{
		}

		template <typename T>
			requires std::predicate<const Predicate&, T&, const AdditionalArgs&...>
		[[nodiscard]]
		constexpr bool matches(
			T& target
		) const noexcept(std::is_nothrow_invocable_v<const Predicate&, T&, const AdditionalArgs&...>)
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

		[[nodiscard]]
		constexpr StringT describe() const
		{
			return std::apply(
				[&, this](auto&... additionalArgs)
				{
					// std::make_format_args requires lvalue-refs, so let's transform rvalue-refs to const lvalue-refs
					constexpr auto makeLvalue = [](auto&& val) noexcept -> const auto& { return val; };
					return format::vformat(
						m_FormatString,
						format::make_format_args(
							makeLvalue(mimicpp::print(additionalArgs))...));
				},
				m_AdditionalArgs);
		}

		[[nodiscard]]
		constexpr auto operator !() const &
			requires std::is_copy_constructible_v<Predicate>
					&& (... && std::is_copy_constructible_v<AdditionalArgs>)
		{
			return make_inverted(
				m_Predicate,
				m_InvertedFormatString,
				m_FormatString,
				std::move(m_AdditionalArgs));
		}

		[[nodiscard]]
		constexpr auto operator !() &&
		{
			return make_inverted(
				std::move(m_Predicate),
				std::move(m_InvertedFormatString),
				std::move(m_FormatString),
				std::move(m_AdditionalArgs));
		}

	private:
		[[no_unique_address]] Predicate m_Predicate;
		StringT m_FormatString;
		StringT m_InvertedFormatString;
		mutable std::tuple<AdditionalArgs...> m_AdditionalArgs{};

		template <typename Fn>
		[[nodiscard]]
		static constexpr auto make_inverted(
			Fn&& fn,
			StringT fmt,
			StringT invertedFmt,
			std::tuple<AdditionalArgs...> tuple
		)
		{
			using NotFnT = decltype(std::not_fn(std::forward<Fn>(fn)));
			return PredicateMatcher<NotFnT, AdditionalArgs...>{
				std::not_fn(std::forward<Fn>(fn)),
				std::move(fmt),
				std::move(invertedFmt),
				std::move(tuple)
			};
		}
	};

	/**
	 * \brief Matcher, which never fails.
	 * \ingroup EXPECTATION_REQUIREMENT
	 * \ingroup EXPECTATION_MATCHER
	 * \snippet Requirements.cpp matcher wildcard
	 */
	class WildcardMatcher
	{
	public:
		static constexpr bool matches(auto&& target) noexcept
		{
			return true;
		}

		static constexpr StringT describe()
		{
			return "has no constraints";
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
	 * \attention Matchers receive their arguments as possibly non-const, which is due to workaround some restrictions
	 * on const qualified views. Either way, matchers should never modify any of their arguments.
	 *
	 * # Matching arguments
	 * In general matchers can be applied via the ``expect::arg<n>`` factory, but they can also be directly used
	 * at the expect statement.
	 * \snippet Requirements.cpp expect::arg
	 * \snippet Requirements.cpp expect arg matcher
	 *
	 * \details For equality testing, there exists an even shorter syntax.
	 * \snippet Requirements.cpp expect arg equal short
	 *
	 * \details Most of the built-in matchers support the inversion operator (``operator !``), which then tests for the opposite
	 * condition.
	 * \snippet Requirements.cpp matcher inverted
	 *
	 * # Custom Matcher
	 * Matchers are highly customizable. In fact, any type which satisfies ``matcher_for`` concept can be used.
	 * There exists no base or interface type, but the ``PredicateMatcher`` servers as a convenient generic type, which
	 * simply contains a predicate, a format string and optional additional arguments. But, this is just one option. If
	 * you have some very specific needs, go and create your matcher from scratch.
	 * \snippet Requirements.cpp matcher predicate matcher
	 *
	 *\{
	 */

	/**
	 * \brief The wildcard matcher, always matching.
	 * \snippet Requirements.cpp matcher wildcard
	 */
	inline constexpr WildcardMatcher _{};

	/**
	 * \brief Tests, whether the target compares equal to the expected value.
	 * \tparam T Expected type.
	 * \param value Expected value.
	 */
	template <typename T>
	[[nodiscard]]
	constexpr auto eq(T&& value)
	{
		return PredicateMatcher{
			std::ranges::equal_to{},
			"== {}",
			"!= {}",
			std::tuple{std::forward<T>(value)}
		};
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
		return PredicateMatcher{
			std::ranges::not_equal_to{},
			"!= {}",
			"== {}",
			std::tuple{std::forward<T>(value)}
		};
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
		return PredicateMatcher{
			std::ranges::less{},
			"< {}",
			">= {}",
			std::tuple{std::forward<T>(value)}
		};
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
		return PredicateMatcher{
			std::ranges::less_equal{},
			"<= {}",
			"> {}",
			std::tuple{std::forward<T>(value)}
		};
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
		return PredicateMatcher{
			std::ranges::greater{},
			"> {}",
			"<= {}",
			std::tuple{std::forward<T>(value)}
		};
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
		return PredicateMatcher{
			std::ranges::greater_equal{},
			">= {}",
			"< {}",
			std::tuple{std::forward<T>(value)}
		};
	}

	/**
	 * \brief Tests, whether the target fulfills the given predicate.
	 * \tparam UnaryPredicate Predicate type.
	 * \param predicate The predicate to test.
	 * \param description The formatting string.
	 * \param invertedDescription The formatting string for the inversion.
	 * \snippet Requirements.cpp matcher predicate
	 */
	template <typename UnaryPredicate>
	[[nodiscard]]
	constexpr auto predicate(
		UnaryPredicate&& predicate,
		StringT description = "passes predicate",
		StringT invertedDescription = "fails predicate"
	)
	{
		return PredicateMatcher{
			std::forward<UnaryPredicate>(predicate),
			std::move(description),
			std::move(invertedDescription),
		};
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
	 * \ingroup EXPECTATION_MATCHER
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
		return PredicateMatcher{
			[](const ViewT target, const ViewT exp)
			{
				return target == exp;
			},
			"is equal to {}",
			"is not equal to {}",
			std::tuple{std::move(expected)}
		};
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
	 * \snippet Requirements.cpp matcher range sorted
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
		return PredicateMatcher{
			[comp = std::move(comparator)]
			<typename Target>(Target&& target, auto& range)  // NOLINT(cppcoreguidelines-missing-std-forward)
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
			"elements are {}",
			"elements are not {}",
			std::tuple{std::views::all(std::forward<Range>(expected))}
		};
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
		return PredicateMatcher{
			[comp = std::move(comparator)]<typename Target
			>(Target&& target, auto& range)  // NOLINT(cppcoreguidelines-missing-std-forward)
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
			"is a permutation of {}",
			"is not a permutation of {}",
			std::tuple{std::views::all(std::forward<Range>(expected))}
		};
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
		return PredicateMatcher{
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
			"is a sorted range",
			"is an unsorted range"
		};
	}

	/**
	 * \brief Tests, whether the target range is empty.
	 */
	[[nodiscard]]
	constexpr auto is_empty()
	{
		return PredicateMatcher{
			[](std::ranges::range auto&& target)
			{
				return std::ranges::empty(target);
			},
			"is an empty range",
			"is not an empty range"
		};
	}

	/**
	 * \brief Tests, whether the target range has the expected size.
	 * \param expected The expected size.
	 */
	[[nodiscard]]
	constexpr auto has_size(const std::integral auto expected)
	{
		return PredicateMatcher{
			[](std::ranges::range auto&& target, const std::integral auto size)
			{
				return std::cmp_equal(
					size,
					std::ranges::size(target));
			},
			"has size of {}",
			"has different size than {}",
			std::tuple{expected}
		};
	}

	/**
	 * \}
	 */
}

#endif
