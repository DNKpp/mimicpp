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
		StringT operator()(T& target) const
		{
			const auto text = mimicpp::print(target);
			return format::vformat(
				m_PredicateDescription,
				std::make_format_args(text));
		}

	private:
		StringT m_PredicateDescription;
	};

	template <typename Predicate, typename Describer, template <typename Derived> typename... Policies>
		requires std::is_move_constructible_v<Predicate>
				&& std::is_move_constructible_v<Describer>
	class PredicateMatcher
		: public Policies<PredicateMatcher<Predicate, Describer, Policies...>>...
	{
	public:
		template <typename OtherPredicate, typename OtherDescriber>
		using RebindT = PredicateMatcher<OtherPredicate, OtherDescriber, Policies...>;

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
			requires std::predicate<const Predicate, T&>
		[[nodiscard]]
		constexpr bool matches(
			T& target
		) const noexcept(std::is_nothrow_invocable_r_v<bool, const Predicate, T&>)
		{
			return std::invoke(
				m_Predicate,
				target);
		}

		template <typename T>
			requires std::regular_invocable<const Describer, T&>
					&& std::convertible_to<std::invoke_result_t<const Describer, T&>, StringT>
		[[nodiscard]]
		constexpr StringT describe(T& target) const
		{
			return std::invoke(
				m_Describer,
				target);
		}

		[[nodiscard]]
		constexpr Predicate&& predicate() && noexcept
		{
			return std::move(m_Predicate);
		}

		[[nodiscard]]
		constexpr Describer&& describer() && noexcept
		{
			return std::move(m_Describer);
		}

	private:
		[[no_unique_address]] Predicate m_Predicate;
		[[no_unique_address]] Describer m_Describer;
	};

	template <template <typename> typename... Policies, typename Predicate, typename Describer>
	[[nodiscard]]
	constexpr PredicateMatcher<Predicate, Describer, Policies...> make_predicate_matcher(
		Predicate predicate,
		Describer describer
	) noexcept(std::is_nothrow_move_constructible_v<Predicate>
				&& std::is_nothrow_move_constructible_v<Describer>)
	{
		return PredicateMatcher<Predicate, Describer, Policies...>{
			std::move(predicate),
			std::move(describer)
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

			auto notPredicate = [predicate = std::move(self).predicate()](auto& target)
			{
				return !std::invoke(predicate, target);
			};

			auto invertedDescriber = [describer = std::move(self).describer()](
				auto& target
			) -> StringT  // NOLINT(bugprone-use-after-move)
			{
				return format::format(
					"!({})",
					std::invoke(describer, target));
			};

			using InvertedMatcherT = typename Derived::template RebindT<decltype(notPredicate), decltype(invertedDescriber)>;
			return InvertedMatcherT{
				std::move(notPredicate),
				std::move(invertedDescriber)
			};
		}
	};
}

namespace mimicpp::matches
{
	inline static const matcher::PredicateMatcher _{
		AlwaysTruePredicate{},
		matcher::TargetPredicateDescriber{"{} without constraints"}
	};

	template <typename T>
	[[nodiscard]]
	constexpr auto eq(T&& value)
	{
		return matcher::make_predicate_matcher<matcher::InvertiblePolicy>(
			std::bind_front(std::ranges::equal_to{}, std::forward<T>(value)),
			matcher::TargetPredicateDescriber{
				format::format("{{}} == {}", mimicpp::print(value))
			});
	}

	template <typename T>
	[[nodiscard]]
	constexpr auto ne(T&& value)
	{
		return matcher::make_predicate_matcher<matcher::InvertiblePolicy>(
			std::bind_front(std::ranges::not_equal_to{}, std::forward<T>(value)),
			matcher::TargetPredicateDescriber{
				format::format("{{}} != {}", mimicpp::print(value))
			});
	}

	template <typename T>
	[[nodiscard]]
	constexpr auto lt(T&& value)
	{
		return matcher::make_predicate_matcher<matcher::InvertiblePolicy>(
			std::bind_front(std::ranges::greater{}, std::forward<T>(value)),
			matcher::TargetPredicateDescriber{
				format::format("{{}} < {}", mimicpp::print(value))
			});
	}

	template <typename T>
	[[nodiscard]]
	constexpr auto le(T&& value)
	{
		return matcher::make_predicate_matcher<matcher::InvertiblePolicy>(
			std::bind_front(std::ranges::greater_equal{}, std::forward<T>(value)),
			matcher::TargetPredicateDescriber{
				format::format("{{}} <= {}", mimicpp::print(value))
			});
	}

	template <typename T>
	[[nodiscard]]
	constexpr auto gt(T&& value)
	{
		return matcher::make_predicate_matcher<matcher::InvertiblePolicy>(
			std::bind_front(std::ranges::less{}, std::forward<T>(value)),
			matcher::TargetPredicateDescriber{
				format::format("{{}} > {}", mimicpp::print(value))
			});
	}

	template <typename T>
	[[nodiscard]]
	constexpr auto ge(T&& value)
	{
		return matcher::make_predicate_matcher<matcher::InvertiblePolicy>(
			std::bind_front(std::ranges::less_equal{}, std::forward<T>(value)),
			matcher::TargetPredicateDescriber{
				format::format("{{}} >= {}", mimicpp::print(value))
			});
	}

	template <typename UnaryPredicate>
	[[nodiscard]]
	constexpr auto predicate(UnaryPredicate&& predicate)
	{
		return matcher::make_predicate_matcher<matcher::InvertiblePolicy>(
			std::forward<UnaryPredicate>(predicate),
			matcher::TargetPredicateDescriber{
				"{} satisfies predicate"
			});
	}
}

#endif
