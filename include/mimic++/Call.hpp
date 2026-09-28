//          Copyright Dominic (DNKpp) Koepke 2024-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_CALL_HPP
#define MIMICPP_CALL_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/TypeTraits.hpp"
#include "mimic++/utilities/SourceLocation.hpp"
#include "mimic++/utilities/Stacktrace.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <cstddef>
    #include <functional>
    #include <tuple>
    #include <type_traits>
#endif

namespace mimicpp::call
{
    template <typename Return, typename... Args>
    class Info
    {
    public:
        using ArgListT = std::tuple<std::reference_wrapper<std::remove_reference_t<Args>>...>;

        ArgListT args;
        ValueCategory fromCategory{};
        Constness fromConstness{};
        util::SourceLocation fromSourceLocation{};
        std::size_t baseStacktraceSkip{};
    };

    template <typename Signature>
    struct info_for_signature
        /** \cond Help doxygen with recursion.*/
        : public info_for_signature<signature_decay_t<Signature>>
    /** \endcond */
    {
    };

    template <typename Signature>
    using info_for_signature_t = typename info_for_signature<Signature>::type;

    template <typename Return, typename... Args>
    struct info_for_signature<Return(Args...)>
    {
        using type = Info<Return, Args...>;
    };

    template <typename Result>
    class ResultStorage
    {
    public:
        [[nodiscard]]
        explicit constexpr ResultStorage(Result&& value) noexcept(std::is_nothrow_move_constructible_v<Result>)
            : m_Value{std::move(value)}
        {
        }

        [[nodiscard]]
        constexpr Result extract() noexcept(std::is_nothrow_move_constructible_v<Result>)
        {
            return std::move(m_Value);
        }

    private:
        Result m_Value;
    };

    template <>
    class ResultStorage<void>
    {
    public:
        static constexpr void extract() noexcept
        {
        }
    };

    template <typename Result>
        requires std::is_reference_v<Result>
    class ResultStorage<Result>
    {
    public:
        using ref = std::reference_wrapper<std::remove_reference_t<Result>>;

        [[nodiscard]]
        explicit constexpr ResultStorage(ref const ref) noexcept
            : m_Ref{ref}
        {
        }

        [[nodiscard]]
        constexpr Result&& extract() noexcept
        {
            return std::forward<Result>(m_Ref.get());
        }

    private:
        ref m_Ref;
    };
}

#endif
