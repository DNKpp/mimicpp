//          Copyright Dominic (DNKpp) Koepke 2024-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_EXPECTATION_REGISTRY_HPP
#define MIMICPP_EXPECTATION_REGISTRY_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/Sequence.hpp"
#include "mimic++/config/Config.hpp"
#include "mimic++/expectation/Expectation.hpp"
#include "mimic++/reporting/GlobalReporter.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <algorithm>
    #include <mutex>
    #include <optional>
    #include <ranges>
    #include <span>
    #include <tuple>
    #include <utility>
    #include <vector>
#endif

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::expectation
{
    /**
     * \brief Central management type for all expectations.
     * \details This type is responsible for actually creating expectations and needs to be informed about incoming calls.
     * Eventually, it needs to get notified when expectations are no longer valid candidates and shall be removed.
     */
    class Registry
    {
    public:
        using Ptr = std::shared_ptr<Registry>;

        /**
         * \brief Deleted copy-constructor.
         */
        Registry(Registry const&) = delete;

        /**
         * \brief Deleted copy-assignment-operator.
         */
        Registry& operator=(Registry const&) = delete;

        /**
         * \brief Deleted move-constructor.
         */
        [[nodiscard]]
        Registry(Registry&&) = delete;

        /**
         * \brief Deleted move-assignment-operator.
         */
        Registry& operator=(Registry&&) = delete;

        /**
         * \brief Defaulted destructor.
         */
        ~Registry() = default;

        /**
         * \brief Defaulted default constructor.
         */
        [[nodiscard]]
        Registry() = default;

        /**
         * \brief Creates a new expectation from the given arguments.
         * \param args The expectation construction arguments.
         */
        template <typename... Args>
            requires std::constructible_from<Expectation, Args...>
        [[nodiscard]]
        Expectation create(Args&&... args)
        {
            std::scoped_lock const lock{m_ExpectationsMx};
            return m_Expectations.emplace_back(std::forward<Args>(args)...);
        }

        /**
         * \brief Removes the given expectation from the internal storage.
         * \param expectation The expectation to be removed.
         * \details This function also checks whether the removed expectation is satisfied.
         * If not, an *unfulfilled expectation*- report is emitted.
         * \attention Removing an expectation, which is not an element of this Registry, is undefined behavior.
         */
        void remove(Expectation const& expectation)
        {
            std::scoped_lock const lock{m_ExpectationsMx};

            auto const iter = std::ranges::find(m_Expectations, expectation);
            MIMICPP_ASSERT(iter != m_Expectations.cend(), "Expectation does not belong to this registry.");
            m_Expectations.erase(iter);

            if (!expectation.is_satisfied())
            {
                reporting::detail::report_unfulfilled_expectation(expectation.report());
            }
        }

        /**
         * \brief Handles the incoming call.
         * \param target The mock-target, which received the call.
         * \param call The call to be handled.
         * \return Returns an appropriate result from the matched expectation.
         * \details This function queries all stored expectations, whether they accept the call.
         * If multiple matches are possible, the best match is selected and a "matched"-report is emitted.
         * If no matches are found, "no matched"-report is emitted and the call is aborted (e.g. by throwing an exception or terminating).
         * If matches are possible, but all expectations are saturated, an "inapplicable match"-report is emitted.
         */
        template <typename Signature>
        [[nodiscard]]
        signature_return_type_t<Signature> handle_call(reporting::TargetReport target, call::info_for_signature_t<Signature> call)
        {
            std::scoped_lock const lock{m_ExpectationsMx};
            auto&& [matches, inapplicableMatches, noMatches] = evaluate_expectations(target, call);

            std::size_t const stacktraceSkip{1u + call.baseStacktraceSkip};
            if (!std::ranges::empty(matches))
            {
                return handle_matched_call<Signature>(std::move(target), std::move(call), matches, stacktraceSkip + 1u);
            }

            if (!std::ranges::empty(inapplicableMatches))
            {
                auto reports = inapplicableMatches | std::views::transform([](auto const& exp) { return exp->report(); });
                reporting::detail::report_inapplicable_matches(
                    reporting::make_call_report(std::move(target), std::move(call), util::stacktrace::current(stacktraceSkip)),
                    std::vector(reports.begin(), reports.end()));
            }

            reporting::detail::report_no_matches(
                reporting::make_call_report(std::move(target), std::move(call), util::stacktrace::current(stacktraceSkip)),
                make_no_match_reports(std::move(noMatches)));
        }

    private:
        std::vector<Expectation> m_Expectations{};
        std::mutex m_ExpectationsMx{};

        [[nodiscard]]
        static std::optional<reporting::RequirementOutcomes> try_match_call(
            reporting::TargetReport const& target,
            auto const& callInfo,
            Expectation const& expectation) noexcept
        {
            try
            {
                return expectation.matches(callInfo);
            }
            catch (...)
            {
                reporting::detail::report_unhandled_exception(
                    reporting::make_call_report(
                        target,
                        callInfo,
                        util::stacktrace::current(3u + callInfo.baseStacktraceSkip)),
                    expectation.report(),
                    std::current_exception());
            }

            return std::nullopt;
        }

        [[nodiscard]]
        auto evaluate_expectations(reporting::TargetReport const& target, auto const& callInfo)
        {
            std::vector<Expectation*> matches{};
            std::vector<Expectation const*> inapplicableMatches{};
            std::vector<std::tuple<Expectation const*, reporting::RequirementOutcomes>> noMatches{};

            for (auto& exp : std::views::reverse(m_Expectations))
            {
                if (std::optional result = try_match_call(target, callInfo, exp))
                {
                    if (std::ranges::any_of(result->outcomes, [](auto const& el) { return el == false; }))
                    {
                        noMatches.emplace_back(&exp, *std::move(result));
                    }
                    else if (!exp.is_applicable())
                    {
                        inapplicableMatches.emplace_back(&exp);
                    }
                    else
                    {
                        matches.emplace_back(&exp);
                    }
                }
            }

            return std::make_tuple(std::move(matches), std::move(inapplicableMatches), std::move(noMatches));
        }

        template <typename Signature>
        [[nodiscard]]
        signature_return_type_t<Signature> handle_matched_call(
            reporting::TargetReport&& target,
            call::info_for_signature_t<Signature>&& callInfo,
            std::span<Expectation* const> const matches,
            std::size_t const stacktraceSkip)
        {
            std::vector reports = std::invoke([&] {
                auto view = std::views::transform(matches, [](auto const& exp) { return exp->report(); });
                return std::vector(view.begin(), view.end());
            });
            MIMICPP_ASSERT(matches.size() == reports.size(), "Size mismatch.");
            auto const bestMatchIter = std::ranges::max_element(
                reports,
                std::not_fn(&sequence::detail::has_better_rating),
                [](auto const& el) noexcept -> const auto& {
                    return std::get<reporting::state_applicable>(el.controlReport).sequenceRatings;
                });
            auto& expectation = *matches[std::ranges::distance(reports.cbegin(), bestMatchIter)];

            if (settings::report_success())
            {
                // Todo: Avoid the call copy
                // Maybe we can prevent the copy here, but we should keep the instruction order as-is,
                // because in cases of a throwing finalizer, we might introduce bugs.
                // At least there are some tests, which will fail if done incorrectly.
                reporting::detail::report_full_match(
                    reporting::make_call_report(std::move(target), callInfo, util::stacktrace::current(stacktraceSkip)),
                    std::move(*bestMatchIter));
            }

            return expectation.consume(callInfo);
        }

        [[nodiscard]]
        static std::vector<reporting::NoMatchReport> make_no_match_reports(std::vector<std::tuple<Expectation const*, reporting::RequirementOutcomes>>&& outcomes)
        {
            std::vector<reporting::NoMatchReport> reports{};
            reports.reserve(outcomes.size());
            for (auto&& [expectationPtr, outcome] : std::move(outcomes))
            {
                reports.emplace_back(expectationPtr->report(), std::move(outcome));
            }

            return reports;
        }
    };
}

#endif
