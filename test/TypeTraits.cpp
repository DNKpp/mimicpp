// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/TypeTraits.hpp"

#include <catch2/catch_template_test_macros.hpp>

#include <concepts>

namespace
{
	template <typename>
	struct signature_helper;

	template <typename Return, typename... Args>
	struct signature_helper<Return(Args...)>
	{
		using sig_noexcept = Return(Args...) noexcept;
	};

	template <typename Return, typename... Args>
	struct signature_helper<Return(Args..., ...)>
	{
		using sig_noexcept = Return(Args..., ...) noexcept;
	};
}

TEMPLATE_TEST_CASE_SIG(
	"signature_add_noexcept adds noexcept qualifier if not already present.",
	"[type_traits]",
	((bool dummy, typename Expected, typename Signature), dummy, Expected, Signature),
	(true, void() noexcept, void()),
	(true, void(int) noexcept, void(int)),
	(true, void(...) noexcept, void(...)),
	(true, void(float, int) noexcept, void(float, int)),
	(true, void(float, ...) noexcept, void(float, ...)),
	(true, double() noexcept, double()),
	(true, double(int) noexcept, double(int)),
	(true, double(...) noexcept, double(...)),
	(true, double(float, int) noexcept, double(float, int)),
	(true, double(float, ...) noexcept, double(float, ...))
)
{
	STATIC_REQUIRE(std::same_as<Expected, typename mimicpp::signature_add_noexcept<Signature>::type>);
	STATIC_REQUIRE(std::same_as<Expected, mimicpp::signature_add_noexcept_t<Signature>>);

	using SignatureNoexceptT = typename signature_helper<Signature>::sig_noexcept;
	STATIC_REQUIRE(std::same_as<Expected, typename mimicpp::signature_add_noexcept<SignatureNoexceptT>::type>);
	STATIC_REQUIRE(std::same_as<Expected, mimicpp::signature_add_noexcept_t<SignatureNoexceptT>>);
}
