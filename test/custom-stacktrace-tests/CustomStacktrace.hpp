// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_TESTS_CUSTOM_STACKTRACE_HPP
#define MIMICPP_TESTS_CUSTOM_STACKTRACE_HPP

#pragma once

#include "mimic++/Stacktrace.hpp"

// ReSharper disable CppUnusedIncludeDirective
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_templated.hpp>
#include <catch2/trompeloeil.hpp>
// ReSharper restore CppUnusedIncludeDirective

#include "../unit-tests/TestTypes.hpp"

class CustomBackend
{
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

    inline static InvocableMock<std::size_t, const BackendT&> sizeMock{};

    [[nodiscard]]
    static std::size_t size(const BackendT& backend)
    {
        return sizeMock.Invoke(backend);
    }

    inline static InvocableMock<bool, const BackendT&> emptyMock{};

    [[nodiscard]]
    static bool empty(const BackendT& backend)
    {
        return emptyMock.Invoke(backend);
    }

    inline static InvocableMock<std::string, const BackendT&, std::size_t> descriptionMock{};

    [[nodiscard]]
    static std::string description(const BackendT& backend, const std::size_t at)
    {
        return descriptionMock.Invoke(backend, at);
    }

    inline static InvocableMock<std::string, const BackendT&, std::size_t> sourceMock{};

    [[nodiscard]]
    static std::string source_file(const BackendT& backend, const std::size_t at)
    {
        return sourceMock.Invoke(backend, at);
    }

    inline static InvocableMock<std::size_t, const BackendT&, std::size_t> lineMock{};

    [[nodiscard]]
    static std::size_t source_line(const BackendT& backend, const std::size_t at)
    {
        return lineMock.Invoke(backend, at);
    }
};

#endif
