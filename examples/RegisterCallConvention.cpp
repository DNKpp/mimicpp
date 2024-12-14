// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/CallConvention.hpp"
#include "mimic++/InterfaceMock.hpp"

#include <catch2/catch_test_macros.hpp>

//! [register __stdcall]
MIMICPP_REGISTER_CALL_CONVENTION(__stdcall, detail::stdcall_convention);

//! [register __stdcall]

TEST_CASE(
    "Mock supports signatures with explicit call-convention.",
    "[example][example::call-convention]")
{
    //! [mock __stdcall]
    mimicpp::Mock<void __stdcall()> mock{}; // the call-convention must be registered
    SCOPED_EXP mock.expect_call();          // define expectations as usual
                                            // note: ``expect_call`` still uses the default call-convention
    mock();                                 // as usual, but this time, the compiler uses the __stdcall convention
    //! [mock __stdcall]
}

TEST_CASE(
    "Interface mocks supports signatures with explicit call-convention.",
    "[example][example::call-convention]")
{
    //! [mock interface __stdcall]
    struct interface
    {
        virtual ~interface() = default;
        virtual void __stdcall foo() const = 0; // note the explicit call-convention specification
    };

    struct derived
        : public interface
    {
        MIMICPP_MOCK_METHOD(foo, void, (), const, __stdcall); // the call-convention goes last
    };

    derived mock{};
    SCOPED_EXP mock.foo_.expect_call(); // as usual for interface mocks: the mock has a ``_`` suffix
    mock.foo();
    //! [mock interface __stdcall]
}
