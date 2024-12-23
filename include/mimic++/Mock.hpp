// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_MOCK_HPP
#define MIMICPP_MOCK_HPP

#pragma once

#include "mimic++/Expectation.hpp"
#include "mimic++/ExpectationBuilder.hpp"
#include "mimic++/Fwd.hpp"
#include "mimic++/Stacktrace.hpp"
#include "mimic++/TypeTraits.hpp"
#include "mimic++/Utility.hpp"
#include "mimic++/policies/GeneralPolicies.hpp"

namespace mimicpp::detail
{
    template <typename Derived, typename Signature>
    using call_interface_t = typename call_convention_traits<
        signature_call_convention_t<Signature>>::template call_interface_t<Derived, Signature>;

    template <typename Derived, typename Signature, typename... Params>
    class DefaultCallInterface<
        Derived,
        Signature,
        Constness::non_const,
        ValueCategory::any,
        type_list<Params...>>
    {
    public:
        constexpr signature_return_type_t<Signature> operator()(
            Params... params,
            const std::source_location& from = std::source_location::current()) noexcept(signature_is_noexcept_v<Signature>)
        {
            return static_cast<const Derived&>(*this)
                .handle_call(std::tuple{std::ref(params)...}, from);
        }
    };

    template <typename Derived, typename Signature, typename... Params>
    class DefaultCallInterface<
        Derived,
        Signature,
        Constness::as_const,
        ValueCategory::any,
        type_list<Params...>>
    {
    public:
        constexpr signature_return_type_t<Signature> operator()(
            Params... params,
            const std::source_location& from = std::source_location::current()) const noexcept(signature_is_noexcept_v<Signature>)
        {
            return static_cast<const Derived&>(*this)
                .handle_call(std::tuple{std::ref(params)...}, from);
        }
    };

    template <typename Derived, typename Signature, typename... Params>
    class DefaultCallInterface<
        Derived,
        Signature,
        Constness::non_const,
        ValueCategory::lvalue,
        type_list<Params...>>
    {
    public:
        constexpr signature_return_type_t<Signature> operator()(
            Params... params,
            const std::source_location& from = std::source_location::current()) & noexcept(signature_is_noexcept_v<Signature>)
        {
            return static_cast<const Derived&>(*this)
                .handle_call(std::tuple{std::ref(params)...}, from);
        }
    };

    template <typename Derived, typename Signature, typename... Params>
    class DefaultCallInterface<
        Derived,
        Signature,
        Constness::as_const,
        ValueCategory::lvalue,
        type_list<Params...>>
    {
    public:
        constexpr signature_return_type_t<Signature> operator()(
            Params... params,
            const std::source_location& from = std::source_location::current()) const& noexcept(signature_is_noexcept_v<Signature>)
        {
            return static_cast<const Derived&>(*this)
                .handle_call(std::tuple{std::ref(params)...}, from);
        }
    };

    template <typename Derived, typename Signature, typename... Params>
    class DefaultCallInterface<
        Derived,
        Signature,
        Constness::non_const,
        ValueCategory::rvalue,
        type_list<Params...>>
    {
    public:
        constexpr signature_return_type_t<Signature> operator()(
            Params... params,
            const std::source_location& from = std::source_location::current()) && noexcept(signature_is_noexcept_v<Signature>)
        {
            return static_cast<const Derived&>(*this)
                .handle_call(std::tuple{std::ref(params)...}, from);
        }
    };

    template <typename Derived, typename Signature, typename... Params>
    class DefaultCallInterface<
        Derived,
        Signature,
        Constness::as_const,
        ValueCategory::rvalue,
        type_list<Params...>>
    {
    public:
        constexpr signature_return_type_t<Signature> operator()(
            Params... params,
            const std::source_location& from = std::source_location::current()) const&& noexcept(signature_is_noexcept_v<Signature>)
        {
            return static_cast<const Derived&>(*this)
                .handle_call(std::tuple{std::ref(params)...}, from);
        }
    };

    template <
        typename Derived,
        typename Signature,
        Constness constQualifier = signature_const_qualification_v<Signature>,
        ValueCategory refQualifier = signature_ref_qualification_v<Signature>,
        typename ParamList = signature_param_list_t<Signature>>
    class MockFrontend;

    template <typename Derived, typename Signature, typename... Params>
    class MockFrontend<
        Derived,
        Signature,
        Constness::non_const,
        ValueCategory::any,
        type_list<Params...>>
    {
    public:
        template <typename... Args>
            requires(... && requirement_for<Args, Params>)
        [[nodiscard]]
        constexpr auto expect_call(Args&&... args)
        {
            return static_cast<const Derived&>(*this)
                .make_expectation_builder(std::forward<Args>(args)...);
        }
    };

    template <typename Derived, typename Signature, typename... Params>
    class MockFrontend<
        Derived,
        Signature,
        Constness::as_const,
        ValueCategory::any,
        type_list<Params...>>
    {
    public:
        template <typename... Args>
            requires(... && requirement_for<Args, Params>)
        [[nodiscard]]
        constexpr auto expect_call(Args&&... args) const
        {
            return static_cast<const Derived&>(*this)
                .make_expectation_builder(std::forward<Args>(args)...);
        }
    };

    template <typename Derived, typename Signature, typename... Params>
    class MockFrontend<
        Derived,
        Signature,
        Constness::non_const,
        ValueCategory::lvalue,
        type_list<Params...>>
    {
    public:
        template <typename... Args>
            requires(... && requirement_for<Args, Params>)
        [[nodiscard]]
        constexpr auto expect_call(Args&&... args) &
        {
            return static_cast<const Derived&>(*this)
                .make_expectation_builder(std::forward<Args>(args)...);
        }
    };

    template <typename Derived, typename Signature, typename... Params>
    class MockFrontend<
        Derived,
        Signature,
        Constness::as_const,
        ValueCategory::lvalue,
        type_list<Params...>>
    {
    public:
        template <typename... Args>
            requires(... && requirement_for<Args, Params>)
        [[nodiscard]]
        constexpr auto expect_call(Args&&... args) const&
        {
            return static_cast<const Derived&>(*this)
                .make_expectation_builder(std::forward<Args>(args)...);
        }
    };

    template <typename Derived, typename Signature, typename... Params>
    class MockFrontend<
        Derived,
        Signature,
        Constness::non_const,
        ValueCategory::rvalue,
        type_list<Params...>>
    {
    public:
        template <typename... Args>
            requires(... && requirement_for<Args, Params>)
        [[nodiscard]]
        constexpr auto expect_call(Args&&... args) &&
        {
            return static_cast<const Derived&>(*this)
                .make_expectation_builder(std::forward<Args>(args)...);
        }
    };

    template <typename Derived, typename Signature, typename... Params>
    class MockFrontend<
        Derived,
        Signature,
        Constness::as_const,
        ValueCategory::rvalue,
        type_list<Params...>>
    {
    public:
        template <typename... Args>
            requires(... && requirement_for<Args, Params>)
        [[nodiscard]]
        constexpr auto expect_call(Args&&... args) const&&
        {
            return static_cast<const Derived&>(*this)
                .make_expectation_builder(std::forward<Args>(args)...);
        }
    };

    template <typename Signature>
    using expectation_collection_ptr_for = std::shared_ptr<ExpectationCollection<signature_decay_t<Signature>>>;

    template <typename Signature, typename ParamList = signature_param_list_t<Signature>>
    class BasicMock;

    template <typename Signature, typename... Params>
    class BasicMock<Signature, type_list<Params...>>
        : public MockFrontend<
              // MockFrontend doesn't need to know about the call-convention, thus remove it
              BasicMock<Signature, type_list<Params...>>,
              signature_remove_call_convention_t<Signature>>,
          public call_interface_t<
              BasicMock<Signature, type_list<Params...>>,
              Signature>
    {
        using SignatureT = signature_remove_call_convention_t<Signature>;

        friend class MockFrontend<BasicMock, SignatureT>;
        friend call_interface_t<BasicMock, Signature>;

        static constexpr Constness constQualification = signature_const_qualification_v<SignatureT>;
        static constexpr ValueCategory refQualification = signature_ref_qualification_v<SignatureT>;

    protected:
        using ExpectationCollectionPtrT = expectation_collection_ptr_for<SignatureT>;

        [[nodiscard]]
        explicit BasicMock(
            ExpectationCollectionPtrT collection,
            const std::size_t stacktraceSkip) noexcept
            : m_Expectations{std::move(collection)},
              m_StacktraceSkip{stacktraceSkip + 2u} // skips the operator() and the handle_call from the stacktrace
        {
        }

    private:
        ExpectationCollectionPtrT m_Expectations;
        std::size_t m_StacktraceSkip;

        [[nodiscard]]
        constexpr signature_return_type_t<SignatureT> handle_call(
            std::tuple<std::reference_wrapper<std::remove_reference_t<Params>>...>&& params,
            const std::source_location& from) const
        {
            return m_Expectations->handle_call(
                call::info_for_signature_t<SignatureT>{
                    .args = std::move(params),
                    .fromCategory = refQualification,
                    .fromConstness = constQualification,
                    .fromSourceLocation = from,
                    .stacktrace = stacktrace::current(m_StacktraceSkip)});
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
    struct expectation_collection_factory<type_list<UniqueSignatures...>>
    {
        [[nodiscard]]
        static auto make()
        {
            return std::tuple{
                std::make_shared<ExpectationCollection<UniqueSignatures>>()...};
        }
    };

    template <typename FirstSignature, typename... OtherSignatures>
    Mock<FirstSignature, OtherSignatures...> make_interface_mock();
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
            : Mock{0u}
        {
        }

        /**
         * \brief Constructor, initializing the base-stacktrace-skip.
         * \param baseStacktraceSkip The base-stacktrace-skip.
         * \details This sets up the base-stacktrace-skip, which will be used when generating stacktraces for the
         * invocations. This can be used, to eliminate irrelevant or noise stacktrace-entries from the top.
         * \note This sets just the base value. Internally this value will be further increased.
         */
        [[nodiscard]]
        explicit Mock(const std::size_t baseStacktraceSkip)
            : Mock{
                  detail::expectation_collection_factory<
                      detail::unique_list_t<
                          signature_decay_t<FirstSignature>,
                          signature_decay_t<OtherSignatures>...>>::make(),
                  baseStacktraceSkip}
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
        [[nodiscard]]
        explicit Mock(
            std::tuple<Collections...> collections,
            const std::size_t stacktraceSkip) noexcept
            : detail::BasicMock<FirstSignature>{
                  std::get<detail::expectation_collection_ptr_for<FirstSignature>>(collections),
                  stacktraceSkip},
              // clang-format off
              detail::BasicMock<OtherSignatures>{
                  std::get<detail::expectation_collection_ptr_for<OtherSignatures>>(collections),
                  stacktraceSkip}...
        // clang-format on
        {
        }
    };

    /**
     * \}
     */
}

#endif
