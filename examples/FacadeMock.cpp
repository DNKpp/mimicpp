//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Facade.hpp"
#include "mimic++/policies/FinalizerPolicies.hpp"

TEST_CASE(
    "Facade-Mocks can have an explicit *this* param.",
    "[example][example::mock][example::mock::facade]")
{
    //! [facade mock with this]
    namespace finally = mimicpp::finally;

    class Base
    {
    public:
        virtual ~Base() = default;

        virtual int foo() const
        {
            return 42;
        }
    };

    class Derived
        : public Base
    {
    public:
        ~Derived() override = default;

        // This alias is required because the `..._WITH_THIS` macros are not able to determine the current type by themselves.
        using self_type = Derived;

        // This generates the override method and a mock object named foo_, which expects an explicit *this* param.
        // => `Mock<int(Derived const*) const>`
        MAKE_MEMBER_MOCK_WITH_THIS(foo, int, (), const override);
    };

    Derived object{};

    // The first param behaves like an actual *this* pointer.
    SCOPED_EXP object.foo_.expect_call(&object)
        // Let's redirect the call to the actual `Base::foo` implementation.
        and finally::returns_apply_all_result_of([](auto* self) { return self->Base::foo(); });

    CHECK(42 == object.foo());
    //! [facade mock with this]
}
