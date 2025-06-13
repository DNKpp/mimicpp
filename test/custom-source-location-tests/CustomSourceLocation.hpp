//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_TESTS_CUSTOM_SOURCE_LOCATION_HPP
#define MIMICPP_TESTS_CUSTOM_SOURCE_LOCATION_HPP

#pragma once

#include "../unit-tests/TrompeloeilExt.hpp"
#include "mimic++/Fwd.hpp"

#include <memory>
#include <string_view>

class CustomBackend
{
public:
    class Inner
    {
    public:
        InvocableMock<std::string_view> functionNameMock{};
        InvocableMock<std::string_view> fileNameMock{};
        InvocableMock<std::size_t> lineMock{};
        InvocableMock<std::size_t> columnMock{};
    };

    std::shared_ptr<Inner> inner{};
};

// This may already exist, when set as config option. But, we want to test with this particular backend.
#ifdef MIMICPP_CONFIG_ALTERNATIVE_SOURCE_LOCATION_BACKEND
    #undef MIMICPP_CONFIG_ALTERNATIVE_SOURCE_LOCATION_BACKEND
#endif

#define MIMICPP_CONFIG_ALTERNATIVE_SOURCE_LOCATION_BACKEND CustomBackend

template <>
struct mimicpp::util::source_location::backend_traits<CustomBackend>
{
    using BackendT = CustomBackend;

    inline static InvocableMock<CustomBackend> currentMock{};

    [[nodiscard]]
    static BackendT current() noexcept
    {
        return currentMock.Invoke();
    }

    [[nodiscard]]
    static std::string_view file_name(BackendT const& backend)
    {
        return backend.inner->fileNameMock.Invoke();
    }

    [[nodiscard]]
    static std::string_view function_name(BackendT const& backend)
    {
        return backend.inner->functionNameMock.Invoke();
    }

    [[nodiscard]]
    static std::size_t line(BackendT const& backend)
    {
        return backend.inner->lineMock.Invoke();
    }

    [[nodiscard]]
    static std::size_t column(BackendT const& backend)
    {
        return backend.inner->columnMock.Invoke();
    }
};

#include "mimic++/utilities/SourceLocation.hpp"

static_assert(mimicpp::util::source_location::backend<CustomBackend>);

#endif
