// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/TypeTraits.hpp"

#include <catch2/catch_template_test_macros.hpp>

#include <optional>

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

TEMPLATE_TEST_CASE(
	"signature_add_noexcept adds noexcept qualifier if not already present.",
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
	using ExpectedT = typename signature_helper<TestType>::sig_noexcept;

	STATIC_REQUIRE(std::same_as<ExpectedT, typename mimicpp::signature_add_noexcept<TestType>::type>);
	STATIC_REQUIRE(std::same_as<ExpectedT, mimicpp::signature_add_noexcept_t<TestType>>);

	STATIC_REQUIRE(std::same_as<ExpectedT, typename mimicpp::signature_add_noexcept<ExpectedT>::type>);
	STATIC_REQUIRE(std::same_as<ExpectedT, mimicpp::signature_add_noexcept_t<ExpectedT>>);
}

TEMPLATE_TEST_CASE(
	"signature_remove_noexcept removes noexcept qualifier if present.",
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

	SECTION("Function with c-ellipsis.")
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

TEMPLATE_TEST_CASE_SIG(
	"is_overloadable_with determines, whether the first signature is overloadable with the second one.",
	"[type_traits]",
	((bool expected, typename First, typename Second), expected, First, Second),
	(false, void(), void()),
	(false, void() const, void() const),
	(false, void() &, void() &),
	(false, void() const &, void() const &),
	(false, void() &&, void() &&),
	(false, void() const &&, void() const &&),

	(false, int(), void()),
	(false, int() const, void() const),
	(false, int() &, void() &),
	(false, int() const &, void() const &),
	(false, int() &&, void() &&),
	(false, int() const &&, void() const &&),

	(false, void(), void() &),
	(false, void(), void() const &),
	(false, void(), void() &&),
	(false, void(), void() const &&),

	(false, void() const, void() &),
	(false, void() const, void() const &),
	(false, void() const, void() &&),
	(false, void() const, void() const &&),

	(true, void(), void() const),
	(true, void() &, void() const &),
	(true, void() &, void() &&),
	(true, void() &, void() const &&),
	(true, void() const &, void() &),
	(true, void() const &, void() &&),
	(true, void() const &, void() const &&),
	(true, void() &&, void() &),
	(true, void() &&, void() const &),
	(true, void() &&, void() const &&),
	(true, void() const &&, void() &),
	(true, void() const &&, void() const &),
	(true, void() const &&, void() &&),

	(true, void(), void(int)),
	(true, int(), void(int)),
	(true, void(int), void(short))
)
{
	STATIC_REQUIRE(expected == mimicpp::is_overloadable_with<First, Second>::value);
	STATIC_REQUIRE(expected == mimicpp::is_overloadable_with_v<First, Second>);
	STATIC_REQUIRE(expected == mimicpp::is_overloadable_with<Second, First>::value);
	STATIC_REQUIRE(expected == mimicpp::is_overloadable_with_v<Second, First>);

	using mimicpp::signature_add_noexcept_t;

	STATIC_REQUIRE(expected == mimicpp::is_overloadable_with<signature_add_noexcept_t<First>, Second>::value);
	STATIC_REQUIRE(expected == mimicpp::is_overloadable_with_v<signature_add_noexcept_t<First>, Second>);
	STATIC_REQUIRE(expected == mimicpp::is_overloadable_with<Second, signature_add_noexcept_t<First>>::value);
	STATIC_REQUIRE(expected == mimicpp::is_overloadable_with_v<Second, signature_add_noexcept_t<First>>);

	STATIC_REQUIRE(expected == mimicpp::is_overloadable_with<First, signature_add_noexcept_t<Second>>::value);
	STATIC_REQUIRE(expected == mimicpp::is_overloadable_with_v<First, signature_add_noexcept_t<Second>>);
	STATIC_REQUIRE(expected == mimicpp::is_overloadable_with<signature_add_noexcept_t<Second>, First>::value);
	STATIC_REQUIRE(expected == mimicpp::is_overloadable_with_v<signature_add_noexcept_t<Second>, First>);

	STATIC_REQUIRE(expected == mimicpp::is_overloadable_with<signature_add_noexcept_t<First>, signature_add_noexcept_t<Second>>::value);
	STATIC_REQUIRE(expected == mimicpp::is_overloadable_with_v<signature_add_noexcept_t<First>, signature_add_noexcept_t<Second>>);
	STATIC_REQUIRE(expected == mimicpp::is_overloadable_with<signature_add_noexcept_t<Second>, signature_add_noexcept_t<First>>::value);
	STATIC_REQUIRE(expected == mimicpp::is_overloadable_with_v<signature_add_noexcept_t<Second>, signature_add_noexcept_t<First>>);
}

TEMPLATE_TEST_CASE_SIG(
	"is_overload_set determines, whether given signatures are valid overloads of each other.",
	"[type_traits]",
	((bool expected, typename First, typename... Others), expected, First, Others...),
	(false, void(), int()),
	(false, void(), void() const &),
	(false, void() &, void() const &, int () &, void() &&, void() const &&),

	(true, void()),
	(true, void(), void() const),
	(true, void() &, void() const &, void() &&, void() const &&),
	(true, short (int), void() &, void() const &, void() &&, void() const &&)
)
{
	STATIC_REQUIRE(expected == mimicpp::is_overload_set<First, Others...>::value);
	STATIC_REQUIRE(expected == mimicpp::is_overload_set_v<First, Others...>);

	STATIC_REQUIRE(expected == mimicpp::is_overload_set<Others..., First>::value);
	STATIC_REQUIRE(expected == mimicpp::is_overload_set_v<Others..., First>);
}
