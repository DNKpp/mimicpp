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
		using sig = Return(Args...);
		using sig_noexcept = Return(Args...) noexcept;
	};

	template <typename Return, typename... Args>
	struct signature_helper<Return(Args..., ...)>
	{
		using sig = Return(Args..., ...);
		using sig_noexcept = Return(Args..., ...) noexcept;
	};

	template <typename Return, typename... Args>
	struct signature_helper<Return(Args...) const>
	{
		using sig = Return(Args...) const;
		using sig_noexcept = Return(Args...) const noexcept;
	};

	template <typename Return, typename... Args>
	struct signature_helper<Return(Args..., ...) const>
	{
		using sig = Return(Args..., ...) const;
		using sig_noexcept = Return(Args..., ...) const noexcept;
	};

	template <typename Return, typename... Args>
	struct signature_helper<Return(Args...) &>
	{
		using sig = Return(Args...) &;
		using sig_noexcept = Return(Args...) & noexcept;
	};

	template <typename Return, typename... Args>
	struct signature_helper<Return(Args..., ...) &>
	{
		using sig = Return(Args..., ...) &;
		using sig_noexcept = Return(Args..., ...) & noexcept;
	};

	template <typename Return, typename... Args>
	struct signature_helper<Return(Args...) const &>
	{
		using sig = Return(Args...) const &;
		using sig_noexcept = Return(Args...) const & noexcept;
	};

	template <typename Return, typename... Args>
	struct signature_helper<Return(Args..., ...) const &>
	{
		using sig = Return(Args..., ...) const &;
		using sig_noexcept = Return(Args..., ...) const & noexcept;
	};

	template <typename Return, typename... Args>
	struct signature_helper<Return(Args...) &&>
	{
		using sig = Return(Args...) &&;
		using sig_noexcept = Return(Args...) && noexcept;
	};

	template <typename Return, typename... Args>
	struct signature_helper<Return(Args..., ...) &&>
	{
		using sig = Return(Args..., ...) &&;
		using sig_noexcept = Return(Args..., ...) && noexcept;
	};

	template <typename Return, typename... Args>
	struct signature_helper<Return(Args...) const &&>
	{
		using sig = Return(Args...) const &&;
		using sig_noexcept = Return(Args...) const && noexcept;
	};

	template <typename Return, typename... Args>
	struct signature_helper<Return(Args..., ...) const &&>
	{
		using sig = Return(Args..., ...) const &&;
		using sig_noexcept = Return(Args..., ...) const && noexcept;
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

TEMPLATE_TEST_CASE(
	"signature_remove_noexcept removes noexcept qualifier present.",
	"[type_traits]",
	void(),
	void(int),
	void(...),
	void(float, int),
	void(float, ...),
	double(),
	double(int),
	double(...),
	double(float, int),
	double(float, ...),

	void() const,
	void(int) const,
	void(...) const,
	void(float, int) const,
	void(float, ...) const,
	double() const,
	double(int) const,
	double(...) const,
	double(float, int) const,
	double(float, ...) const,

	void() &,
	void(int) &,
	void(...) &,
	void(float, int) &,
	void(float, ...) &,
	double() &,
	double(int) &,
	double(...) &,
	double(float, int) &,
	double(float, ...) &,

	void() const &,
	void(int) const &,
	void(...) const &,
	void(float, int) const &,
	void(float, ...) const &,
	double() const &,
	double(int) const &,
	double(...) const &,
	double(float, int) const &,
	double(float, ...) const &,

	void() &&,
	void(int) &&,
	void(...) &&,
	void(float, int) &&,
	void(float, ...) &&,
	double() &&,
	double(int) &&,
	double(...) &&,
	double(float, int) &&,
	double(float, ...) &&,

	void() const &&,
	void(int) const &&,
	void(...) const &&,
	void(float, int) const &&,
	void(float, ...) const &&,
	double() const &&,
	double(int) const &&,
	double(...) const &&,
	double(float, int) const &&,
	double(float, ...) const &&
)
{
	using SignatureT = TestType;
	STATIC_REQUIRE(std::same_as<SignatureT, typename mimicpp::signature_remove_noexcept<SignatureT>::type>);
	STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_remove_noexcept_t<SignatureT>>);

	using SignatureNoexceptT = typename signature_helper<SignatureT>::sig_noexcept;
	STATIC_REQUIRE(std::same_as<SignatureT, typename mimicpp::signature_remove_noexcept<SignatureNoexceptT>::type>);
	STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_remove_noexcept_t<SignatureNoexceptT>>);
}

TEMPLATE_TEST_CASE_SIG(
	"signature_decay removes all modifiers.",
	"[type_traits]",
	((bool dummy, typename Return, typename... Args), dummy, Return, Args...),
	(true, void),
	(true, void, int),
	(true, void, float, int),
	(true, void, float&),
	(true, void, const float&),
	(true, void, float&&),
	(true, void, const float&&),
	(true, void, float*),
	(true, void, const float*),

	(true, double),
	(true, double, int),
	(true, double, float, int),
	(true, double, float&),
	(true, double, const float&),
	(true, double, float&&),
	(true, double, const float&&),
	(true, double, float*),
	(true, double, const float*),

	(true, double&),
	(true, double&, int),
	(true, double&, float, int),
	(true, double&, float&),
	(true, double&, const float&),
	(true, double&, float&&),
	(true, double&, const float&&),
	(true, double&, float*),
	(true, double&, const float*),

	(true, const double&),
	(true, const double&, int),
	(true, const double&, float, int),
	(true, const double&, float&),
	(true, const double&, const float&),
	(true, const double&, float&&),
	(true, const double&, const float&&),
	(true, const double&, float*),
	(true, const double&, const float*),

	(true, double&&),
	(true, double&&, int),
	(true, double&&, float, int),
	(true, double&&, float&),
	(true, double&&, const float&),
	(true, double&&, float&&),
	(true, double&&, const float&&),
	(true, double&&, float*),
	(true, double&&, const float*),

	(true, const double&&),
	(true, const double&&, int),
	(true, const double&&, float, int),
	(true, const double&&, float&),
	(true, const double&&, const float&),
	(true, const double&&, float&&),
	(true, const double&&, const float&&),
	(true, const double&&, float*),
	(true, const double&&, const float*),

	(true, void*),
	(true, void*, int),
	(true, void*, float, int),
	(true, void*, float&),
	(true, void*, const float&),
	(true, void*, float&&),
	(true, void*, const float&&),
	(true, void*, float*),
	(true, void*, const float*),

	(true, const void*),
	(true, const void*, int),
	(true, const void*, float, int),
	(true, const void*, float&),
	(true, const void*, const float&),
	(true, const void*, float&&),
	(true, const void*, const float&&),
	(true, const void*, float*),
	(true, const void*, const float*)
)
{
	SECTION("Variadic c++ function.")
	{
		using SignatureT = Return(Args...);
		STATIC_REQUIRE(std::same_as<SignatureT, typename mimicpp::signature_decay<Return(Args...)>::type>);
		STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args...)>>);

		STATIC_REQUIRE(std::same_as<SignatureT, typename mimicpp::signature_decay<Return(Args...) const>::type>);
		STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args...) const>>);

		STATIC_REQUIRE(std::same_as<SignatureT, typename mimicpp::signature_decay<Return(Args...) &>::type>);
		STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args...) &>>);

		STATIC_REQUIRE(std::same_as<SignatureT, typename mimicpp::signature_decay<Return(Args...) const &>::type>);
		STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args...) const &>>);

		STATIC_REQUIRE(std::same_as<SignatureT, typename mimicpp::signature_decay<Return(Args...) &&>::type>);
		STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args...) &&>>);

		STATIC_REQUIRE(std::same_as<SignatureT, typename mimicpp::signature_decay<Return(Args...) const &&>::type>);
		STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args...) const &&>>);

		STATIC_REQUIRE(std::same_as<SignatureT, typename mimicpp::signature_decay<Return(Args...) noexcept>::type>);
		STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args...) noexcept>>);

		STATIC_REQUIRE(std::same_as<SignatureT, typename mimicpp::signature_decay<Return(Args...) const noexcept>::type>);
		STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args...) const noexcept>>);

		STATIC_REQUIRE(std::same_as<SignatureT, typename mimicpp::signature_decay<Return(Args...) & noexcept>::type>);
		STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args...) & noexcept>>);

		STATIC_REQUIRE(std::same_as<SignatureT, typename mimicpp::signature_decay<Return(Args...) const & noexcept>::type>);
		STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args...) const & noexcept>>);

		STATIC_REQUIRE(std::same_as<SignatureT, typename mimicpp::signature_decay<Return(Args...) && noexcept>::type>);
		STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args...) && noexcept>>);

		STATIC_REQUIRE(std::same_as<SignatureT, typename mimicpp::signature_decay<Return(Args...) const && noexcept>::type>);
		STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args...) const && noexcept>>);
	}

	SECTION("Function with c-elipsis.")
	{
		using SignatureT = Return(Args..., ...);

		STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args..., ...)>>);
		STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args..., ...) const>>);
		STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args..., ...) &>>);
		STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args..., ...) const &>>);
		STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args..., ...) &&>>);
		STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args..., ...) const &&>>);
		STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args..., ...) noexcept>>);
		STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args..., ...) const noexcept>>);
		STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args..., ...) & noexcept>>);
		STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args..., ...) const & noexcept>>);
		STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args..., ...) && noexcept>>);
		STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args..., ...) const && noexcept>>);
	}
}

TEMPLATE_TEST_CASE_SIG(
	"signature_return_type extracts the return type from the given signature.",
	"[type_traits]",
	((bool dummy, typename Expected, typename Signature), dummy, Expected, Signature),
	(true, void, void()),
	(true, void, void(int)),
	(true, void, void(...)),
	(true, void, void(float, int)),
	(true, void, void(float, ...)),

	(true, double, double()),
	(true, double, double(int)),
	(true, double, double(...)),
	(true, double, double(float, int)),
	(true, double, double(float, ...)),

	(true, double&, double&()),
	(true, double&, double&(int)),
	(true, double&, double&(...)),
	(true, double&, double&(float, int)),
	(true, double&, double&(float, ...)),

	(true, const double&, const double&()),
	(true, const double&, const double&(int)),
	(true, const double&, const double&(...)),
	(true, const double&, const double&(float, int)),
	(true, const double&, const double&(float, ...)),

	(true, double&&, double&&()),
	(true, double&&, double&&(int)),
	(true, double&&, double&&(...)),
	(true, double&&, double&&(float, int)),
	(true, double&&, double&&(float, ...)),

	(true, const double&&, const double&&()),
	(true, const double&&, const double&&(int)),
	(true, const double&&, const double&&(...)),
	(true, const double&&, const double&&(float, int)),
	(true, const double&&, const double&&(float, ...)),

	(true, void*, void*()),
	(true, void*, void*(int)),
	(true, void*, void*(...)),
	(true, void*, void*(float, int)),
	(true, void*, void*(float, ...)),

	(true, const void*, const void*()),
	(true, const void*, const void*(int)),
	(true, const void*, const void*(...)),
	(true, const void*, const void*(float, int)),
	(true, const void*, const void*(float, ...))
)
{
	STATIC_REQUIRE(
		std::same_as<
		Expected,
		typename mimicpp::signature_return_type<Signature>::type>);
	STATIC_REQUIRE(
		std::same_as<
		Expected,
		mimicpp::signature_return_type_t<Signature>>);

	STATIC_REQUIRE(
		std::same_as<
		Expected,
		typename mimicpp::signature_return_type<mimicpp::signature_add_noexcept_t<Signature>>::type>);
	STATIC_REQUIRE(
		std::same_as<
		Expected,
		mimicpp::signature_return_type_t<mimicpp::signature_add_noexcept_t<Signature>>>);
}

TEMPLATE_TEST_CASE_SIG(
	"signature_param_type extracts the n-th param type of the given signature.",
	"[type_traits]",
	((typename Expected, std::size_t index, typename Signature), Expected, index, Signature),
	(int, 0, void(int)),
	(int, 0, void(int, double)),
	(double, 1, void(int, double))
)
{
	STATIC_REQUIRE(
		(
		std::same_as<
		Expected,
		typename mimicpp::signature_param_type<index,Signature>::type>));
	STATIC_REQUIRE(
		(
		std::same_as<
		Expected,
		mimicpp::signature_param_type_t<index, Signature>>));

	STATIC_REQUIRE(
		(
		std::same_as<
		Expected,
		typename mimicpp::signature_param_type<index, mimicpp::signature_add_noexcept_t<Signature>>::type>));
	STATIC_REQUIRE(
		(
		std::same_as<
		Expected,
		mimicpp::signature_param_type_t<index, mimicpp::signature_add_noexcept_t<Signature>>>));
}
