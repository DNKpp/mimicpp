// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_MOCK_HPP
#define MIMICPP_MOCK_HPP

#pragma once

#include "mimic++/Expectation.hpp"
#include "mimic++/ExpectationBuilder.hpp"
#include "mimic++/ExpectationPolicies.hpp"
#include "mimic++/Utility.hpp"

namespace mimicpp::detail
{
    template <typename Derived>
    [[nodiscard]]
    constexpr const Derived& as_derived(auto& self) noexcept
    {
        return static_cast<const Derived&>(self);
    }

    template <
        typename Derived,
        typename SignatureWithoutNoexcept,
        bool isNoexcept>
    class MockFrontend;

    template <
        typename Derived,
        typename Return,
        typename... Params,
        bool isNoexcept>
    class MockFrontend<Derived, Return(Params...), isNoexcept>
    {
    public:
        constexpr Return operator()(
            Params... params,
            const std::source_location& from = std::source_location::current()) noexcept(isNoexcept)
        {
            return as_derived<Derived>(*this)
                .handle_call(std::tuple{std::ref(params)...}, from);
        }

        template <typename... Args>
            requires(... && requirement_for<Args, Params>)
        [[nodiscard]] constexpr auto expect_call(Args&&... args)
        {
            return as_derived<Derived>(*this)
                .make_expectation_builder(std::forward<Args>(args)...);
        }
    };

    template <
        typename Derived,
        typename Return,
        typename... Params,
        bool isNoexcept>
    class MockFrontend<Derived, Return(Params...) const, isNoexcept>
    {
    public:
        constexpr Return operator()(
            Params... params,
            const std::source_location& from = std::source_location::current()) const noexcept(isNoexcept)
        {
            return as_derived<Derived>(*this)
                .handle_call(std::tuple{std::ref(params)...}, from);
        }

        template <typename... Args>
            requires(... && requirement_for<Args, Params>)
        [[nodiscard]] constexpr auto expect_call(Args&&... args) const
        {
            return as_derived<Derived>(*this)
                .make_expectation_builder(std::forward<Args>(args)...);
        }
    };

    template <
        typename Derived,
        typename Return,
        typename... Params,
        bool isNoexcept>
    class MockFrontend<Derived, Return(Params...)&, isNoexcept>
    {
    public:
        constexpr Return operator()(
            Params... params,
            const std::source_location& from = std::source_location::current()) & noexcept(isNoexcept)
        {
            return as_derived<Derived>(*this)
                .handle_call(std::tuple{std::ref(params)...}, from);
        }

        template <typename... Args>
            requires(... && requirement_for<Args, Params>)
        [[nodiscard]] constexpr auto expect_call(Args&&... args) &
        {
            return as_derived<Derived>(*this)
                .make_expectation_builder(std::forward<Args>(args)...);
        }
    };

    template <
        typename Derived,
        typename Return,
        typename... Params,
        bool isNoexcept>
    class MockFrontend<Derived, Return(Params...) const&, isNoexcept>
    {
    public:
        constexpr Return operator()(
            Params... params,
            const std::source_location& from = std::source_location::current()) const& noexcept(isNoexcept)
        {
            return as_derived<Derived>(*this)
                .handle_call(std::tuple{std::ref(params)...}, from);
        }

        template <typename... Args>
            requires(... && requirement_for<Args, Params>)
        [[nodiscard]] constexpr auto expect_call(Args&&... args) const&
        {
            return as_derived<Derived>(*this)
                .make_expectation_builder(std::forward<Args>(args)...);
        }
    };

    template <
        typename Derived,
        typename Return,
        typename... Params,
        bool isNoexcept>
    class MockFrontend<Derived, Return(Params...)&&, isNoexcept>
    {
    public:
        constexpr Return operator()(
            Params... params,
            const std::source_location& from = std::source_location::current()) && noexcept(isNoexcept)
        {
            return as_derived<Derived>(*this)
                .handle_call(std::tuple{std::ref(params)...}, from);
        }

        template <typename... Args>
            requires(... && requirement_for<Args, Params>)
        [[nodiscard]] constexpr auto expect_call(Args&&... args) &&
        {
            return as_derived<Derived>(*this)
                .make_expectation_builder(std::forward<Args>(args)...);
        }
    };

    template <
        typename Derived,
        typename Return,
        typename... Params,
        bool isNoexcept>
    class MockFrontend<Derived, Return(Params...) const&&, isNoexcept>
    {
    public:
        constexpr Return operator()(
            Params... params,
            const std::source_location& from = std::source_location::current()) const&& noexcept(isNoexcept)
        {
            return as_derived<Derived>(*this)
                .handle_call(std::tuple{std::ref(params)...}, from);
        }

        template <typename... Args>
            requires(... && requirement_for<Args, Params>)
        [[nodiscard]] constexpr auto expect_call(Args&&... args) const&&
        {
            return as_derived<Derived>(*this)
                .make_expectation_builder(std::forward<Args>(args)...);
        }
    };

    template <typename Signature>
    using expectation_collection_ptr_for = std::shared_ptr<ExpectationCollection<signature_decay_t<Signature>>>;

    template <typename Signature, typename ParamList = signature_param_list_t<Signature>>
    class BasicMock;

    template <typename Signature, typename... Params>
    class BasicMock<Signature, std::tuple<Params...>>
        : public MockFrontend<
              BasicMock<Signature, std::tuple<Params...>>,
              signature_remove_noexcept_t<Signature>,
              signature_is_noexcept_v<Signature>>
    {
        friend class MockFrontend<
            BasicMock,
            signature_remove_noexcept_t<Signature>,
            signature_is_noexcept_v<Signature>>;

        using SignatureT = Signature;
        using ReturnT = signature_return_type_t<SignatureT>;
        static constexpr Constness constQualification = signature_const_qualification_v<SignatureT>;
        static constexpr ValueCategory refQualification = signature_ref_qualification_v<SignatureT>;

    protected:
        using ExpectationCollectionPtrT = expectation_collection_ptr_for<Signature>;

        [[nodiscard]]
        explicit BasicMock(ExpectationCollectionPtrT collection) noexcept
            : m_Expectations{std::move(collection)}
        {
        }

    private:
        ExpectationCollectionPtrT m_Expectations;

        [[nodiscard]]
        constexpr ReturnT handle_call(
            std::tuple<std::reference_wrapper<std::remove_reference_t<Params>>...>&& params,
            const std::source_location& from) const
        {
            return m_Expectations->handle_call(
                call::info_for_signature_t<SignatureT>{
                    .args = std::move(params),
                    .fromCategory = refQualification,
                    .fromConstness = constQualification,
                    .fromSourceLocation = from});
        }

        template <typename... Args>
        [[nodiscard]]
        constexpr auto make_expectation_builder(Args&&... args) const
        {
            return detail::make_expectation_builder(m_Expectations, std::forward<Args>(args)...)
                && expectation_policies::Category<refQualification>{}
                && expectation_policies::Constness<constQualification>{};
        }
    };

    template <typename List>
    struct expectation_collection_factory;

    template <typename... UniqueSignatures>
    struct expectation_collection_factory<std::tuple<UniqueSignatures...>>
    {
        [[nodiscard]]
        static auto make()
        {
            return std::tuple{
                std::make_shared<ExpectationCollection<UniqueSignatures>>()...};
        }
    };
}

namespace mimicpp
{
    /**
     * \defgroup MOCK mock
     * \brief The core aspect of the library.
     * \details Mocks are responsible for providing a convenient interface to set up expectations and handle received calls.
     * At a basic level users can specify arbitrary overload sets, which the mock shall provide and for which expectations can be defined.
     *
     * Mocks themselves can be used as public members and can therefore serve as member function mocks. Have a look at the following example,
     * which demonstrates how one is able to test a custom stack container adapter (like ``std::stack``) by utilizing mocking.
     *
     * At first, we define a simple concept, which our mock must satisfy.
     * \snippet Mock.cpp stack concept
     * The implemented must be test-able for emptiness, must have a ``push_back`` and ``pop_back`` function and must provide access to the
     * last element (both, const and non-const).
     *
     * The ``MyStack`` implementation is rather simple. It provides a pair of ``pop`` and ``push`` functions and exposes the top element;
     * as const or non-const reference.
     * ``pop`` and both ``top`` overloads test whether the inner container is empty and throw conditionally.
     * \snippet Mock.cpp stack adapter
     *
     * To make the test simpler, let's fixate ``T`` as ``int``.
     * A conforming mock then must provide 5 member functions:
     *	* ``bool empty() const``,
     *	* ``void push_back(int)``,
     *	* ``void pop_back()``,
     *	* ``int& back()`` and
     *	* ``const int& back() const``.
     * \snippet Mock.cpp container mock
     * As you can see, mimicpp::Mock accepts any valid signature and even supports overloading (as shown by ``back``).
     *
     * Eventually we are able to formulate our tests. The test for ``push`` is rather straight-forward.
     * \snippet Mock.cpp test push
     * We create our container mock, setting up the expectation and moving it into the actual stack.
     * The move is required, because ``MyStack`` doesn't expose the inner container. There are more advanced solutions for that kind of design,
     * but that would be out of scope of this example.
     *
     * The ``pop()`` test is also quite simple, but we should definitely test, that ``pop`` never tries to remove an element from an empty container.
     * \snippet Mock.cpp test pop
     * As you can see, the success test sets up two distinct expectations; They may be satisfied in any order, even if only one order here semantically
     * makes sense. We could use a ``sequence`` object here instead, but with the second test (the empty case) we already have enough confidence.
     *
     * The empty test then just creates a single expectation, but in fact it also implicitly tests, that the ``pop_back`` of the container is never called.
     *
     * Finally, we should test both of the ``top()`` functions.
     * \snippet Mock.cpp test top
     * In the first section we check both overloads, when no elements are present.
     *
     * The second section then tests when the container is not empty.
     *
     * \{
     */

    /**
     * \brief A Mock type, which fully supports overload sets.
     * \tparam FirstSignature The first signature.
     * \tparam OtherSignatures Other signatures.
     */
    template <typename FirstSignature, typename... OtherSignatures>
        requires is_overload_set_v<FirstSignature, OtherSignatures...>
    class Mock
        : public detail::BasicMock<FirstSignature>,
          public detail::BasicMock<OtherSignatures>...
    {
    public:
        using detail::BasicMock<FirstSignature>::operator();
        using detail::BasicMock<FirstSignature>::expect_call;
        using detail::BasicMock<OtherSignatures>::operator()...;
        using detail::BasicMock<OtherSignatures>::expect_call...;

        /**
         * \brief Defaulted destructor.
         */
        ~Mock() = default;

        /**
         * \brief Default constructor.
         */
        [[nodiscard]]
        Mock()
            : Mock{
                  detail::expectation_collection_factory<
                      detail::unique_list_t<
                          signature_decay_t<FirstSignature>,
                          signature_decay_t<OtherSignatures>...>>::make()}
        {
        }

        /**
         * \brief Deleted copy constructor.
         */
        Mock(const Mock&) = delete;

        /**
         * \brief Deleted copy assignment operator.
         */
        Mock& operator=(const Mock&) = delete;

        /**
         * \brief Defaulted move constructor.
         */
        [[nodiscard]]
        Mock(Mock&&) = default;

        /**
         * \brief Defaulted move assignment operator.
         */
        Mock& operator=(Mock&&) = default;

    private:
        template <typename... Collections>
        explicit Mock(std::tuple<Collections...> collections) noexcept
            : detail::BasicMock<FirstSignature>{std::get<detail::expectation_collection_ptr_for<FirstSignature>>(collections)},
              detail::BasicMock<OtherSignatures>{std::get<detail::expectation_collection_ptr_for<OtherSignatures>>(collections)}...
        {
        }
    };

    /**
     * \}
     */
}

#endif
