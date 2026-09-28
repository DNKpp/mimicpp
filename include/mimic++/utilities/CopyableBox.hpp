//          Copyright Dominic (DNKpp) Koepke 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_UTILITIES_COPYABLE_BOX_HPP
#define MIMICPP_UTILITIES_COPYABLE_BOX_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/config/Config.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <concepts>
    #include <memory>
    #include <type_traits>
#endif

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::util
{
    /**
     * \brief Simple copyable value type, which owns a heap-allocated value.
     * \ingroup UTILITIES
     * \details
     * Unlike the common `std::unique_ptr`, this type is directly copyable.
     * Besides that, this type is fully `constexpr` (unlike `std::unique_ptr` before C++23).
     */
    template <typename T>
    class CopyableBox
    {
    public:
        constexpr ~CopyableBox() noexcept
        {
            delete m_Data;
        }

        [[nodiscard]]
        constexpr CopyableBox(CopyableBox const& other)
            : m_Data{other.m_Data ? new T{*other} : nullptr}
        {
        }

        [[nodiscard]]
        constexpr CopyableBox(CopyableBox&& other) noexcept
            : m_Data{std::exchange(other.m_Data, nullptr)}
        {
        }

        constexpr CopyableBox& operator=(CopyableBox other) noexcept
        {
            std::ranges::swap(m_Data, other.m_Data);
            return *this;
        }

        [[nodiscard]]
        constexpr CopyableBox()
            : m_Data{new T{}}
        {
        }

        [[nodiscard]]
        explicit(false) constexpr CopyableBox(T const& value)
            : m_Data{new T{value}}
        {
        }

        [[nodiscard]]
        explicit(false) constexpr CopyableBox(T&& value)
            : m_Data{new T{std::move(value)}}
        {
        }

        [[nodiscard]]
        constexpr T& operator*() noexcept
        {
            MIMICPP_ASSERT(m_Data, "Data is null.");
            return *m_Data;
        }

        [[nodiscard]]
        constexpr T const& operator*() const noexcept
        {
            MIMICPP_ASSERT(m_Data, "Data is null.");
            return *m_Data;
        }

        [[nodiscard]]
        constexpr T* operator->() noexcept
        {
            return m_Data;
        }

        [[nodiscard]]
        constexpr T const* operator->() const noexcept
        {
            return m_Data;
        }

        [[nodiscard]]
        explicit constexpr operator bool() const noexcept
        {
            return m_Data != nullptr;
        }

        [[nodiscard]]
        friend constexpr bool operator==(CopyableBox const& lhs, CopyableBox const& rhs)
        {
            return *lhs == *rhs;
        }

    private:
        T* m_Data{};
    };
}

#endif
