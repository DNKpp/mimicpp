//          Copyright Dominic (DNKpp) Koepke 2024-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_CALL_HPP
#define MIMICPP_CALL_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/TypeTraits.hpp"
#include "mimic++/config/Config.hpp"
#include "mimic++/utilities/SourceLocation.hpp"
#include "mimic++/utilities/Stacktrace.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <cstddef>
    #include <functional>
    #include <memory>
    #include <optional>
    #include <tuple>
    #include <type_traits>
    #include <utility>
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

    /**
     * \brief Caller-owned storage, which receives the result of a finalizer.
     * \details The caller creates the slot on its stack and passes its address through the type-erased expectation
     * interface. That way, the result never has to be copied (e.g. into a `std::any`), which makes move-only return-types
     * possible and avoids any dynamic allocation.
     */
    template <typename Result>
    class ResultSlot
    {
    public:
        template <typename... Args>
        constexpr void emplace(Args&&... args)
        {
            MIMICPP_ASSERT(!m_Value, "Slot is already occupied.");
            m_Value.emplace(std::forward<Args>(args)...);
        }

        [[nodiscard]]
        constexpr Result extract() noexcept(std::is_nothrow_move_constructible_v<Result>)
        {
            MIMICPP_ASSERT(m_Value, "Slot is empty.");
            return std::move(*m_Value);
        }

    private:
        std::optional<Result> m_Value{};
    };

    template <>
    class ResultSlot<void>
    {
    public:
        static constexpr void extract() noexcept
        {
        }
    };

    template <typename Result>
        requires std::is_reference_v<Result>
    class ResultSlot<Result>
    {
    public:
        constexpr void emplace(Result&& ref) noexcept
        {
            MIMICPP_ASSERT(!m_Ptr, "Slot is already occupied.");
            m_Ptr = std::addressof(ref);
        }

        [[nodiscard]]
        constexpr Result&& extract() noexcept
        {
            MIMICPP_ASSERT(m_Ptr, "Slot is empty.");
            return std::forward<Result>(*m_Ptr);
        }

    private:
        std::remove_reference_t<Result>* m_Ptr{};
    };
}

#endif
