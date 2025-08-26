//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_TESTS_CUSTOM_STACKTRACE_HPP
#define MIMICPP_TESTS_CUSTOM_STACKTRACE_HPP

#pragma once

#include "mimic++/Stacktrace.hpp"

#include "../unit-tests/TestTypes.hpp"

class CustomBackend
{
public:
    class Inner
    {
    public:
        InvocableMock<std::size_t> sizeMock{};
        InvocableMock<bool> emptyMock{};
        InvocableMock<std::string, std::size_t> descriptionMock{};
        InvocableMock<std::string, std::size_t> sourceMock{};
        InvocableMock<std::size_t, std::size_t> lineMock{};
    };

    std::shared_ptr<Inner> inner{};
};

struct mimicpp::custom::find_stacktrace_backend
{
    using type = CustomBackend;
};

template <>
struct mimicpp::util::stacktrace::backend_traits<CustomBackend>
{
    using BackendT = CustomBackend;

    inline static InvocableMock<CustomBackend, std::size_t> currentMock{};
    inline static InvocableMock<CustomBackend, std::size_t, std::size_t> currentMaxMock{};

    [[nodiscard]]
    static BackendT current(std::size_t const skip, std::size_t const max)
    {
        return currentMaxMock.Invoke(skip, max);
    }

    [[nodiscard]]
    static BackendT current(std::size_t const skip)
    {
        return currentMock.Invoke(skip);
    }
    [[nodiscard]]
    static std::size_t size(BackendT const& backend)
    {
        return backend.inner->sizeMock.Invoke();
    }

    [[nodiscard]]
    static bool empty(BackendT const& backend)
    {
        return backend.inner->emptyMock.Invoke();
    }

    [[nodiscard]]
    static std::string description(BackendT const& backend, std::size_t const at)
    {
        return backend.inner->descriptionMock.Invoke(at);
    }

    [[nodiscard]]
    static std::string source_file(BackendT const& backend, std::size_t const at)
    {
        return backend.inner->sourceMock.Invoke(at);
    }

    [[nodiscard]]
    static std::size_t source_line(BackendT const& backend, std::size_t const at)
    {
        return backend.inner->lineMock.Invoke(at);
    }
};

static_assert(mimicpp::util::stacktrace::backend<CustomBackend>);

#endif
