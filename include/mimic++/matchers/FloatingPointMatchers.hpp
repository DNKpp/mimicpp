//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_MATCHERS_FLOATING_POINT_MATCHERS_HPP
#define MIMICPP_MATCHERS_FLOATING_POINT_MATCHERS_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/config/Config.hpp"
#include "mimic++/matchers/GeneralMatchers.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <algorithm>
    #include <cmath>
    #include <concepts>
    #include <limits>
    #include <stdexcept>
    #include <tuple>
#endif

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::matches
{
    /**
     * \addtogroup MATCHERS
     *
     *\{
     */

    /**
     * \brief Tests, whether the floating-point target is NaN.
     */
    [[nodiscard]]
    consteval auto NaN() noexcept
    {
        return PredicateMatcher{
            []<std::floating_point T>(const T target) noexcept {
                return std::isnan(target);
            },
            "is not a number (NaN)",
            "is a number"};
    }

    namespace detail
    {
        void check_fp_value(const std::floating_point auto value)
        {
            if (std::isnan(value)
                || std::isinf(value))
            {
                throw std::runtime_error{"Value must be not NaN and not infinity."};
            }
        }

        void check_fp_epsilon(const std::floating_point auto epsilon)
        {
            if (std::isnan(epsilon)
                || std::isinf(epsilon)
                || epsilon <= 0.)
            {
                throw std::runtime_error{"Epsilon must be not NaN, not infinity and not less or equal 0."};
            }
        }
    }

    /**
     * \brief Tests, whether the floating-point target is approximately equal to ``value``.
     * \param value The value to compare to.
     * \param epsilon The maximum absolute difference.
     * \throws std::runtime_error When ``value`` is ``NaN`` or ``infinity``.
     * \throws std::runtime_error When ``epsilon`` is ``NaN``, ``infinity`` or non-positive.
     * \return The newly created matcher.
     */
    [[nodiscard]]
    constexpr auto approx_abs(
        const std::floating_point auto value,
        const std::floating_point auto epsilon)
    {
        detail::check_fp_value(value);
        detail::check_fp_epsilon(epsilon);

        return PredicateMatcher{
            [](const std::floating_point auto target, const auto val, const auto eps) {
                return std::abs(target - val) <= eps;
            },
            "is approximately {} +- {}",
            "is not approximately {} +- {}",
            std::make_tuple(value, epsilon)};
    }

    /**
     * \brief Tests, whether the floating-point target is approximately equal to ``value``.
     * \param value The value to compare against.
     * \param relEpsilon The maximum relative difference.
     * \return The newly created matcher.
     * \throws std::runtime_error When ``value`` is ``NaN`` or ``infinity``.
     * \throws std::runtime_error When ``relEpsilon`` is ``NaN``, ``infinity`` or non-positive.
     *
     * \details This functions compares both floating-point values with a scaled epsilon.
     * In fact:
     *
     * ``|a-b| <= eps * max(|a|, |b|)``,
     *
     * where ``a`` and ``b`` are the operands and ``eps`` denotes the factor of the maximum input by which
     * ``a`` and ``b`` may differ.
     * \note This algorithm was published by [Donald Knuth](https://en.wikipedia.org/wiki/Donald_Knuth) in his book
     * “The Art of Computer Programming, Volume II: Seminumerical Algorithms (Addison-Wesley, 1969)”.
     */
    [[nodiscard]]
    constexpr auto approx_rel(
        const std::floating_point auto value,
        const std::floating_point auto relEpsilon)
    {
        detail::check_fp_value(value);
        detail::check_fp_epsilon(relEpsilon);

        return PredicateMatcher{
            [](const std::floating_point auto target, const auto val, const auto rel) {
                // when target equals +-infinity, that leads to an inf epsilon, which is not very useful
                if (!std::isinf(target))
                {
                    const auto absDiff = std::abs(target - val);
                    const auto scaledEpsilon = rel * std::max(std::abs(target), std::abs(val));
                    return absDiff <= scaledEpsilon;
                }

                return false;
            },
            "is approximately {} +- ({} * max(|lhs|, |rhs|))",
            "is not approximately {} +- ({} * max(|lhs|, |rhs|))",
            std::make_tuple(value, relEpsilon)};
    }

    /**
     * \brief Tests, whether the floating-point target is approximately equal to ``value``.
     * \param value The value to compare against.
     * \return The newly created matcher.
     * \throws std::runtime_error When ``value`` is ``NaN`` or ``infinity``.
     *
     * \details This overload sets ``100 * std::numeric_limits<Float>::epsilon()`` as the relative epsilon value.
     * This seems like a reasonable choice, which is also used by the ``catch2``'s ``WithinRel`` matcher.
     * \see https://github.com/catchorg/Catch2/blob/devel/docs/comparing-floating-point-numbers.md#withinrel
     * \details For detailed information about the underlying algorithm, have a look at the primary overload.
     * \see approx_rel(std::floating_point auto, std::floating_point auto)
     */
    template <std::floating_point Float>
    [[nodiscard]]
    constexpr auto approx_rel(const Float value)
    {
        return matches::approx_rel(
            value,
            std::numeric_limits<Float>::epsilon() * Float{100});
    }

    /**
     * \}
     */
}

#endif
