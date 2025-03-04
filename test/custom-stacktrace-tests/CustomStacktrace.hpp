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
struct mimicpp::stacktrace::backend_traits<CustomBackend>
{
    using BackendT = CustomBackend;

    inline static InvocableMock<CustomBackend, std::size_t> currentMock{};

    [[nodiscard]]
    static BackendT current(const std::size_t skip)
    {
        return currentMock.Invoke(skip);
    }

    [[nodiscard]]
    static std::size_t size(const BackendT& backend)
    {
        return backend.inner->sizeMock.Invoke();
    }

    [[nodiscard]]
    static bool empty(const BackendT& backend)
    {
        return backend.inner->emptyMock.Invoke();
    }

    [[nodiscard]]
    static std::string description(const BackendT& backend, const std::size_t at)
    {
        return backend.inner->descriptionMock.Invoke(at);
    }

    [[nodiscard]]
    static std::string source_file(const BackendT& backend, const std::size_t at)
    {
        return backend.inner->sourceMock.Invoke(at);
    }

    [[nodiscard]]
    static std::size_t source_line(const BackendT& backend, const std::size_t at)
    {
        return backend.inner->lineMock.Invoke(at);
    }
};

static_assert(mimicpp::stacktrace::backend<CustomBackend>);

#endif
