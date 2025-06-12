//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

template <typename Return, typename... Params>
class InvocableMockBase;

template <typename Return, typename... Params>
    requires(0u == sizeof...(Params))
class InvocableMockBase<Return, Params...>
{
public:
    MAKE_MOCK0(Invoke, Return(Params...));
    MAKE_CONST_MOCK0(Invoke, Return(Params...));
};

template <typename Return, typename... Params>
    requires(1u == sizeof...(Params))
class InvocableMockBase<Return, Params...>
{
public:
    MAKE_MOCK1(Invoke, Return(Params...));
    MAKE_CONST_MOCK1(Invoke, Return(Params...));
};

template <typename Return, typename... Params>
    requires(2u == sizeof...(Params))
class InvocableMockBase<Return, Params...>
{
public:
    MAKE_MOCK2(Invoke, Return(Params...));
    MAKE_CONST_MOCK2(Invoke, Return(Params...));
};

template <typename Return, typename... Params>
    requires(3u == sizeof...(Params))
class InvocableMockBase<Return, Params...>
{
public:
    MAKE_MOCK3(Invoke, Return(Params...));
    MAKE_CONST_MOCK3(Invoke, Return(Params...));
};

template <typename Return, typename... Params>
    requires(4u == sizeof...(Params))
class InvocableMockBase<Return, Params...>
{
public:
    MAKE_MOCK4(Invoke, Return(Params...));
    MAKE_CONST_MOCK4(Invoke, Return(Params...));
};

template <typename Return, typename... Params>
    requires(5u == sizeof...(Params))
class InvocableMockBase<Return, Params...>
{
public:
    MAKE_MOCK5(Invoke, Return(Params...));
    MAKE_CONST_MOCK5(Invoke, Return(Params...));
};

template <typename Return, typename... Params>
    requires(6u == sizeof...(Params))
class InvocableMockBase<Return, Params...>
{
public:
    MAKE_MOCK6(Invoke, Return(Params...));
    MAKE_CONST_MOCK6(Invoke, Return(Params...));
};

template <typename Return, typename... Params>
    requires(7u == sizeof...(Params))
class InvocableMockBase<Return, Params...>
{
public:
    MAKE_MOCK7(Invoke, Return(Params...));
    MAKE_CONST_MOCK7(Invoke, Return(Params...));
};

template <typename Return, typename... Params>
    requires(8u == sizeof...(Params))
class InvocableMockBase<Return, Params...>
{
public:
    MAKE_MOCK8(Invoke, Return(Params...));
    MAKE_CONST_MOCK8(Invoke, Return(Params...));
};

template <typename Return, typename... Params>
    requires(9u == sizeof...(Params))
class InvocableMockBase<Return, Params...>
{
public:
    MAKE_MOCK9(Invoke, Return(Params...));
    MAKE_CONST_MOCK9(Invoke, Return(Params...));
};

template <typename Return, typename... Params>
    requires(10u == sizeof...(Params))
class InvocableMockBase<Return, Params...>
{
public:
    MAKE_MOCK10(Invoke, Return(Params...));
    MAKE_CONST_MOCK10(Invoke, Return(Params...));
};

template <typename Return, typename... Params>
    requires(11u == sizeof...(Params))
class InvocableMockBase<Return, Params...>
{
public:
    MAKE_MOCK11(Invoke, Return(Params...));
    MAKE_CONST_MOCK11(Invoke, Return(Params...));
};

template <typename Return, typename... Params>
    requires(12u == sizeof...(Params))
class InvocableMockBase<Return, Params...>
{
public:
    MAKE_MOCK12(Invoke, Return(Params...));
    MAKE_CONST_MOCK12(Invoke, Return(Params...));
};

template <typename Return, typename... Params>
    requires(13u == sizeof...(Params))
class InvocableMockBase<Return, Params...>
{
public:
    MAKE_MOCK13(Invoke, Return(Params...));
    MAKE_CONST_MOCK13(Invoke, Return(Params...));
};

template <typename Return, typename... Params>
    requires(14u == sizeof...(Params))
class InvocableMockBase<Return, Params...>
{
public:
    MAKE_MOCK14(Invoke, Return(Params...));
    MAKE_CONST_MOCK14(Invoke, Return(Params...));
};

template <typename Return, typename... Params>
    requires(15u == sizeof...(Params))
class InvocableMockBase<Return, Params...>
{
public:
    MAKE_MOCK15(Invoke, Return(Params...));
    MAKE_CONST_MOCK15(Invoke, Return(Params...));
};

template <typename Return, typename... Params>
class InvocableMock
    : public InvocableMockBase<Return, Params...>
{
public:
    using ReturnT = Return;
    using ParamListT = std::tuple<Params...>;
    using InvocableMockBase<Return, Params...>::Invoke;

    constexpr ReturnT operator()(Params... params)
    {
        return Invoke(std::forward<Params>(params)...);
    }

    constexpr ReturnT operator()(Params... params) const
    {
        return Invoke(std::forward<Params>(params)...);
    }
};
