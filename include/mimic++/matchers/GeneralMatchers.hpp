//          Copyright Dominic (DNKpp) Koepke 2024-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_MATCHERS_GENERAL_MATCHERS_HPP
#define MIMICPP_MATCHERS_GENERAL_MATCHERS_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/config/Config.hpp"
#include "mimic++/matchers/Common.hpp"
#include "mimic++/printing/Format.hpp"
#include "mimic++/printing/Fwd.hpp"
#include "mimic++/printing/StatePrinter.hpp"
#include "mimic++/printing/TypePrinter.hpp"
#include "mimic++/utilities/Concepts.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <array>
    #include <any>
    #include <functional>
    #include <tuple>
    #include <type_traits>
    #include <utility>
#endif

namespace mimicpp::detail
{
    template <typename Arg, typename MatchesProjection = std::identity, typename DescribeProjection = printing::PrintFn>
    struct arg_storage
    {
        using matches_reference = std::invoke_result_t<MatchesProjection, const Arg&>;
        using describe_reference = std::invoke_result_t<DescribeProjection, const Arg&>;

        Arg arg;

        decltype(auto) as_matches_arg() const noexcept(std::is_nothrow_invocable_v<MatchesProjection, const Arg&>)
        {
            return std::invoke(MatchesProjection{}, arg);
        }

        decltype(auto) as_describe_arg() const noexcept(std::is_nothrow_invocable_v<DescribeProjection, const Arg&>)
        {
            return std::invoke(DescribeProjection{}, arg);
        }
    };

    template <typename T>
    struct to_arg_storage
    {
        using type = arg_storage<T>;
    };

    template <typename Arg, typename MatchesProjection, typename DescribeProjection>
    struct to_arg_storage<arg_storage<Arg, MatchesProjection, DescribeProjection>>
    {
        using type = arg_storage<Arg, MatchesProjection, DescribeProjection>;
    };

    template <typename T>
    using to_arg_storage_t = typename to_arg_storage<T>::type;
}

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp
{
    /**
     * \brief Generic matcher and the basic building block of most of the built-in matchers.
     * \tparam Predicate The predicate type.
     * \tparam AdditionalArgs Addition argument types.
     * \ingroup MATCHERS
     */
    template <typename Predicate, typename... AdditionalArgs>
        requires std::is_move_constructible_v<Predicate>
              && (... && std::is_move_constructible_v<AdditionalArgs>)
    class PredicateMatcher
    {
    private:
        using storage_t = std::tuple<detail::to_arg_storage_t<AdditionalArgs>...>;
        template <typename T>
        using matches_reference_t = typename detail::to_arg_storage_t<T>::matches_reference;

    public:
        [[nodiscard]]
        explicit constexpr PredicateMatcher(
            Predicate predicate,
            StringViewT fmt,
            StringViewT invertedFmt,
            std::tuple<AdditionalArgs...> additionalArgs = {})
            noexcept(
                std::is_nothrow_move_constructible_v<Predicate>
                && (... && std::is_nothrow_move_constructible_v<AdditionalArgs>))
            : m_Predicate{std::move(predicate)},
              m_FormatString{std::move(fmt)},
              m_InvertedFormatString{std::move(invertedFmt)},
              m_AdditionalArgs{std::move(additionalArgs)}
        {
        }

        template <typename First, typename... Others>
            requires std::predicate<
                const Predicate&,
                First&,
                Others&...,
                matches_reference_t<AdditionalArgs>...>
        [[nodiscard]]
        constexpr bool matches(
            First& first,
            Others&... others) const
            noexcept(
                std::is_nothrow_invocable_v<
                    const Predicate&,
                    First&,
                    Others&...,
                    matches_reference_t<AdditionalArgs>...>)
        {
            return std::apply(
                [&, this](auto&... additionalArgs) {
                    return std::invoke(
                        m_Predicate,
                        first,
                        others...,
                        additionalArgs.as_matches_arg()...);
                },
                m_AdditionalArgs);
        }

        [[nodiscard]]
        constexpr StringT describe() const
        {
            return std::apply(
                [&](auto&... additionalArgs) {
                    std::array<StringT, sizeof...(additionalArgs)> const descriptions{additionalArgs.as_describe_arg()...};

                    return std::invoke(
                        [&]<std::size_t... indices>(std::index_sequence<indices...> const /*seq*/) {
                            return format::vformat(m_FormatString, format::make_format_args(descriptions[indices]...));
                        },
                        std::index_sequence_for<AdditionalArgs...>{});
                },
                m_AdditionalArgs);
        }

        [[nodiscard]]
        constexpr auto operator!() const&
            requires std::is_copy_constructible_v<Predicate>
                  && std::is_copy_constructible_v<storage_t>
        {
            return make_inverted(
                m_Predicate,
                m_InvertedFormatString,
                m_FormatString,
                m_AdditionalArgs);
        }

        [[nodiscard]]
        constexpr auto operator!() &&
        {
            return make_inverted(
                std::move(m_Predicate),
                std::move(m_InvertedFormatString),
                std::move(m_FormatString),
                std::move(m_AdditionalArgs));
        }

    private:
        [[no_unique_address]] Predicate m_Predicate;
        StringViewT m_FormatString;
        StringViewT m_InvertedFormatString;
        storage_t m_AdditionalArgs;

        template <typename Fn>
        [[nodiscard]]
        static constexpr auto make_inverted(
            Fn&& fn,
            StringViewT fmt,
            StringViewT invertedFmt,
            storage_t tuple)
        {
            using NotFnT = decltype(std::not_fn(std::forward<Fn>(fn)));
            return PredicateMatcher<NotFnT, detail::to_arg_storage_t<AdditionalArgs>...>{
                std::not_fn(std::forward<Fn>(fn)),
                std::move(fmt),
                std::move(invertedFmt),
                std::move(tuple)};
        }
    };

    namespace detail
    {
        class CapturedValue
        {
        public:
            template <typename T>
            explicit constexpr CapturedValue(std::in_place_t const /*tag*/, T&& value, format::format_string<format::fallback_formattable_t<T>> fmt = "{}")
                : m_Value{std::forward<T>(value)},
                  m_Fmt{fmt.get()},
                  m_PrintStrategy{&CapturedValue::print_to<std::remove_cvref_t<T>>}
            {
            }

            using OutIter = std::ostreambuf_iterator<char>;
            OutIter print_to(OutIter out) const // NOLINT(*-use-nodiscard)
            {
                MIMICPP_ASSERT(m_PrintStrategy, "Null print strategy.");
                MIMICPP_ASSERT(m_Value.has_value(), "Null capture.");

                return m_PrintStrategy(std::move(out), m_Fmt, m_Value);
            }

        private:
            std::any m_Value;
            format::vformat_string m_Fmt;
            using PrintStrategy = OutIter(*)(OutIter, format::vformat_string const&, std::any const&);
            PrintStrategy m_PrintStrategy;

            template <typename T>
            static OutIter print_to(OutIter out, format::vformat_string const& fmt, std::any const& capture)
            {
                auto formatter = format::fallback_formattable(std::any_cast<std::remove_cvref_t<T>>(capture));
                return format::vformat_to(
                    std::move(out),
                    fmt,
                    format::make_format_args(formatter));
            }
        };
    }

    template <util::unqualified... Args>
    class MatchEvaluationContext
    {
        using CapturedValue = detail::CapturedValue;
        using CaptureSink = std::back_insert_iterator<std::vector<CapturedValue>>;

    public:
        using expectation_refs = std::tuple<std::unwrap_reference_t<Args> const&...>;

        [[nodiscard]]
        explicit constexpr MatchEvaluationContext(expectation_refs expectations, CaptureSink captureSink)
            : m_Expectations{std::move(expectations)},
              m_captureSink{std::move(captureSink)}
        {
        }

        [[nodiscard]]
        constexpr expectation_refs expectations() const noexcept
        {
            return m_Expectations;
        }

        template <typename T>
        void capture(T&& value)
        {
            *m_captureSink++ = CapturedValue{std::in_place, std::forward<T>(value)};
        }

    private:
        expectation_refs m_Expectations;
        CaptureSink m_captureSink;
    };

    template <bool isInverted, util::unqualified Predicate, util::unqualified... Args>
        requires std::is_move_constructible_v<Predicate>
              && (... && std::is_move_constructible_v<Args>)
    class GenericMatcher
    {
    public:
        using Context = MatchEvaluationContext<Args...>;
        using ArgsStorage = std::tuple<Args...>;

        [[nodiscard]]
        explicit constexpr GenericMatcher(Predicate predicate, format::vformat_string fmt, ArgsStorage args)
            noexcept(
                std::is_nothrow_move_constructible_v<Predicate>
                && std::is_nothrow_move_constructible_v<ArgsStorage>)
            : m_Predicate{std::move(predicate)},
              m_FormatString{std::move(fmt)},
              m_Args{std::move(args)}
        {
        }

        template <typename First, typename... Others>
            requires std::predicate<Predicate const&, Context&, First&, Others&...>
        [[nodiscard]]
        expectation::MatchResult matches(First& first, Others&... others) const
        {
            std::vector<detail::CapturedValue> captures{};

            StringStreamT ss{};
            describe_to(std::ostreambuf_iterator{ss});

            if (Context ctx{m_Args, std::back_inserter(captures)};
                !isInverted == std::invoke(m_Predicate, ctx, first, others...))
            {
                return expectation::MatchSuccess{.description = std::move(ss).str()};
            }

            if (!std::ranges::empty(captures))
            {
                ss << ", but actually ";
                auto iter = captures.cbegin();
                iter->print_to(std::ostreambuf_iterator{ss});

                for (++iter; iter != captures.cend(); ++iter)
                {
                    ss << ", ";
                    iter->print_to(std::ostreambuf_iterator{ss});
                }
            }

            return expectation::MatchFailure{.description = std::move(ss).str()};
        }

        [[nodiscard]]
        constexpr auto operator!() const&
            requires std::is_copy_constructible_v<Predicate>
                  && std::is_copy_constructible_v<ArgsStorage>
        {
            return GenericMatcher<!isInverted, Predicate, Args...>{m_Predicate, m_FormatString, m_Args};
        }

        [[nodiscard]]
        constexpr auto operator!() &&
        {
            return GenericMatcher<!isInverted, Predicate, Args...>{std::move(m_Predicate), std::move(m_FormatString), std::move(m_Args)};
        }

    private:
        Predicate m_Predicate;
        format::vformat_string m_FormatString;
        ArgsStorage m_Args;

        template <print_iterator OutIter>
        OutIter describe_to(OutIter out) const
        {
            if constexpr (isInverted)
            {
                out = format::format_to(std::move(out), "not (");
            }

            out = std::apply(
                [&](auto&&... args) {
                    return format::vformat_to(std::move(out), m_FormatString, format::make_format_args(args...));
                },
                std::tuple<format::fallback_formattable_t<Args const&>...>{m_Args});

            if constexpr (isInverted)
            {
                out = format::format_to(std::move(out), ")");
            }

            return out;
        }
    };

    template <typename Predicate, typename... Args>
    [[nodiscard]]
    constexpr auto make_generic_matcher(
        Predicate && predicate,
        format::format_string<format::fallback_formattable_t<Args>...> const formatString,
        Args && ... args)
    {
        using Matcher = GenericMatcher<false, std::remove_cvref_t<Predicate>, std::remove_cvref_t<Args>...>;
        return Matcher{
            std::forward<Predicate>(predicate),
            formatString.get(),
            std::forward_as_tuple(std::forward<Args>(args)...)};
    }

    /**
     * \brief Matcher, which never fails.
     * \ingroup MATCHERS
     * \snippet Requirements.cpp matcher wildcard
     */
    class WildcardMatcher
    {
    public:
        [[nodiscard]]
        static expectation::MatchResult matches(auto&& /*target*/) noexcept
        {
            return expectation::MatchSuccess{};
        }
    };

    /**
     * \brief Matcher, which can be used to disambiguate between similar overloads.
     * \ingroup MATCHERS
     * \snippet Requirements.cpp matcher type
     */
    template <typename T>
    class TypeMatcher
    {
    public:
        template <typename U>
        using is_accepting = std::is_same<T, U>;

        [[nodiscard]]
        static expectation::MatchResult matches(auto&& /*target*/) noexcept
        {
            return expectation::MatchSuccess{};
        }
    };
}

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::matches
{
    /**
     * \defgroup MATCHERS matchers
     * \brief Matchers check various argument properties.
     * \details Matchers can be used to check various argument properties and are highly customizable. In general,
     * they simply compare their arguments with a pre-defined predicate, but also provide a meaningful description.
     *
     * \attention Matchers receive their arguments as possibly non-const, which is due to workaround some restrictions
     * on const qualified views. Either way, matchers should never modify any of their arguments.
     *
     * ### Matching arguments
     * In general matchers can be applied via the ``expect::arg<n>`` factory, but they can also be directly used
     * at the expect statement.
     * \snippet Requirements.cpp expect::arg
     * \snippet Requirements.cpp expect arg matcher
     *
     * \details For equality testing, there exists an even shorter syntax.
     * \snippet Requirements.cpp expect arg equal short
     *
     * \details Most of the built-in matchers support the inversion operator (`operator !`), which then tests for the opposite
     * condition.
     * \snippet Requirements.cpp matcher inverted
     *
     * ### Custom Matcher
     * Matchers are highly customizable. In fact, any type which satisfies `matcher_for` concept can be used.
     * There exists no base or interface type, but the `GenericMatcher` servers as a convenient generic type,
     * which simply contains a predicate, a format string and optional additional arguments.
     *
     * A very straight-forward custom matcher may look like this:
     * \snippet CustomMatcher.cpp matcher custom contains definition
     * \snippet CustomMatcher.cpp matcher custom contains usage
     *
     * In fact, the `GenericMatcher` is very flexible and can most likely tailored to your needs.
     * For example, you can store any additional data.
     * In this case the internal formatter requires the raw-pattern string, but the actual predicate needs a `std::regex`.
     * \snippet CustomMatcher.cpp matcher custom regex definition
     * \snippet CustomMatcher.cpp matcher custom regex usage
     *
     * Variadic matchers are also directly supported.
     * In this case, the matcher requires two inputs and checks whether the sum of both matches the specified value.
     * \snippet CustomMatcher.cpp matcher custom variadic definition
     * \snippet CustomMatcher.cpp matcher custom variadic usage
     *
     * When there are very special needs, users can also just define their own matcher type without any base-class.
     * The only requirement is a single `matches` function, returning a `mimicpp::expectation::MatchResult`.
     * \snippet CustomMatcher.cpp matcher custom standalone definition
     * \snippet CustomMatcher.cpp matcher custom standalone usage
     *
     * #### Legacy matchers
     * Older versions of *mimic++* required matchers to provide a separate `matches` (returning a plain `bool`) and `describe` function.
     * This interface is still fully supported for backwards-compatibility,
     * but its use is discouraged in favor of the single-`matches`-function interface shown above.
     * \snippet CustomMatcher.cpp matcher custom legacy definition
     * \snippet CustomMatcher.cpp matcher custom legacy usage
     *
     *\{
     */

    /**
     * \brief The wildcard matcher, always matching.
     * \snippet Requirements.cpp matcher wildcard
     */
    [[maybe_unused]] inline constexpr WildcardMatcher _{};

    /**
     * \brief Matcher, which can be used as a last resort to disambiguate similar overloads.
     * \tparam T The exact argument type.
     * \snippet Requirements.cpp matcher type
     */
    template <typename T>
    [[maybe_unused]] inline constexpr TypeMatcher<T> type{};

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
            std::equal_to{},
            "== {}",
            "!= {}",
            std::make_tuple(std::forward<T>(value))};
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
            std::not_equal_to{},
            "!= {}",
            "== {}",
            std::make_tuple(std::forward<T>(value))};
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
            std::less{},
            "< {}",
            ">= {}",
            std::make_tuple(std::forward<T>(value))};
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
            std::less_equal{},
            "<= {}",
            "> {}",
            std::make_tuple(std::forward<T>(value))};
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
            std::greater{},
            "> {}",
            "<= {}",
            std::make_tuple(std::forward<T>(value))};
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
            std::greater_equal{},
            ">= {}",
            "< {}",
            std::make_tuple(std::forward<T>(value))};
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
        StringViewT description = "passes predicate",
        StringViewT invertedDescription = "fails predicate")
    {
        return PredicateMatcher{
            std::forward<UnaryPredicate>(predicate),
            std::move(description),
            std::move(invertedDescription),
        };
    }

    /**
     * \brief Tests whether the target is the expected instance.
     * \tparam T Instance type.
     * \param instance The instance to be compared to.
     * \snippet Requirements.cpp matcher instance
     */
    template <util::satisfies<std::is_lvalue_reference> T>
    [[nodiscard]]
    constexpr auto instance(T&& instance) // NOLINT(cppcoreguidelines-missing-std-forward)
    {
        return make_generic_matcher(
            []<typename Other>(auto& ctx, Other const& target) noexcept
                requires std::is_convertible_v<std::remove_cvref_t<T> const volatile*, Other const volatile*>
            {
                auto const& [expected] = ctx.expectations();
                auto* const targetPtr = std::addressof(target);
                ctx.capture(targetPtr);
                return expected == targetPtr;
            },
            "is instance at {}",
            std::addressof(instance));
    }

    /**
     * \}
     */
}

#endif
