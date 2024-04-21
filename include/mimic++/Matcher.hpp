// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_MATCHER_HPP
#define MIMICPP_MATCHER_HPP

#pragma once

#include "mimic++/Printer.hpp"

#include <concepts>
#include <functional>
#include <type_traits>

#include "Utility.hpp"

namespace mimicpp
{
	template <typename T, typename Target>
	concept matcher_for = std::same_as<T, std::remove_cvref_t<T>>
						&& std::movable<T>
						&& std::destructible<T>
						&& requires(const T& matcher, Target&& target)
						{
							{ matcher.matches(target) } -> std::convertible_to<bool>;
							{ matcher.describe(target) } -> std::convertible_to<StringT>;
						};
}

namespace mimicpp::matcher
{
	template <std::movable Predicate, std::movable Describer>
	class PredicateMatcher
	{
	public:
		[[nodiscard]]
		explicit constexpr PredicateMatcher(
			Predicate predicate,
			Describer describer
		) noexcept(std::is_nothrow_move_constructible_v<Predicate>
					&& std::is_nothrow_move_constructible_v<Describer>)
			: m_Predicate{std::move(predicate)},
			m_Describer{std::move(describer)}
		{
		}

		template <typename T>
			requires std::predicate<const Predicate, T>
		[[nodiscard]]
		constexpr bool matches(T& target) const
		{
			return std::invoke(
				m_Predicate,
				target);
		}

		template <typename T>
			requires std::convertible_to<std::invoke_result_t<const Describer, T>, StringT>
		[[nodiscard]]
		constexpr StringT matches(T& target) const
		{
			return std::invoke(
				m_Describer,
				target);
		}

	private:
		[[no_unique_address]] Predicate m_Predicate;
		[[no_unique_address]] Describer m_Describer;
	};

	class TargetPredicateDescriber
	{
	public:
		[[nodiscard]]
		explicit constexpr TargetPredicateDescriber(StringT&& predicateDescription) noexcept
			: m_PredicateDescription{std::move(predicateDescription)}
		{
		}

		template <typename T>
		[[nodiscard]]
		constexpr StringT operator()(T& target)
		{
			return std::format(
				"{} {}",
				mimicpp::print(target),
				m_PredicateDescription);
		}

	private:
		StringT m_PredicateDescription;
	};
}

namespace mimicpp::matches
{
	inline static const matcher::PredicateMatcher _{
		AlwaysTruePredicate{},
		matcher::TargetPredicateDescriber{"== _"}
	};

	template <typename T>
	[[nodiscard]]
	constexpr auto eq(T&& value)
	{
		return matcher::PredicateMatcher{
			std::bind_front(std::equal_to{}, std::forward<T>(value)),
			matcher::TargetPredicateDescriber{
				std::format("== {}", mimicpp::print(value))
			}
		};
	}
}

#endif
