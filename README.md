![logo](https://github.com/user-attachments/assets/312220b7-c966-484f-b531-13721db998de)

# mimic++, a modern and (mostly) macro free mocking framework

### ✅ Quality

<p align="center">
  ░▒▒▓▓██&nbsp;&nbsp;&nbsp;
  <a href="https://github.com/DNKpp/mimicpp/actions/workflows/codeql.yml">
    <img src="https://github.com/DNKpp/mimicpp/actions/workflows/codeql.yml/badge.svg" alt="CodeQL">
  </a>
  &nbsp;&nbsp;
  <a href="https://app.codacy.com/gh/DNKpp/mimicpp/dashboard">
    <img src="https://app.codacy.com/project/badge/Grade/b852271c6e8742fe8a1667e679dc422b" alt="Codacy Grade">
  </a>
  &nbsp;&nbsp;
  <a href="https://app.codacy.com/gh/DNKpp/mimicpp/dashboard">
    <img src="https://app.codacy.com/project/badge/Coverage/b852271c6e8742fe8a1667e679dc422b" alt="Codacy Coverage">
  </a>
  &nbsp;&nbsp;
  <a href="https://codecov.io/gh/DNKpp/mimicpp">
    <img src="https://codecov.io/gh/DNKpp/mimicpp/graph/badge.svg?token=T9EpgyuyUi" alt="Codecov">
  </a>
  &nbsp;&nbsp;
  <a href="https://coveralls.io/github/DNKpp/mimicpp">
    <img src="https://coveralls.io/repos/github/DNKpp/mimicpp/badge.svg" alt="Coveralls">
  </a>
  &nbsp;&nbsp;&nbsp;██▓▓▒▒░
</p>

### 🔗 Developer Hub

Quick access to **documentation, package managers, and community support**.

<p>
  &nbsp;&nbsp;&nbsp;&nbsp;╌ 🛠️ ╍╌&nbsp;&nbsp;
  <a href="https://godbolt.org/z/63nhG1Mx3">
    <img src="https://img.shields.io/badge/try%20it-On%20Compiler%20Explorer-brightgreen?style=flat&logo=compilerexplorer" alt="Compiler Explorer">
  </a>
  &nbsp;&nbsp;
  <a href="https://vcpkg.link/ports/mimicpp">
    <img src="https://img.shields.io/vcpkg/v/mimicpp" alt="vcpkg">
  </a>
</p>

<p>
  &nbsp;&nbsp;&nbsp;&nbsp;╌ 💬 ╍╌&nbsp;&nbsp;
  <a href="https://discord.gg/Gcay9n444E">
    <img src="https://img.shields.io/badge/join-Community%20Discord-%235865F2?style=flat&logo=discord&logoColor=%235865F2" alt="discord">
  </a>
  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
  <a href="https://github.com/DNKpp/mimicpp/discussions/new?category=q-a">
    <img src="https://img.shields.io/badge/ask-On%20Github-%23009999?style=flat&logo=github" alt="discussion">
  </a>
</p>

<p>
  &nbsp;&nbsp;&nbsp;&nbsp;╌ 📖 ╍╌&nbsp;&nbsp;
  <a href="https://dnkpp.github.io/mimicpp/">
    <img src="https://img.shields.io/badge/read_-Online%20Documentation-%239B59B6?style=flat&logo=readme&logoColor=silver" alt="docs">
  </a>
</p>

---

## Table of Contents

* [Introduction](#introduction)
  * [Core Design](#core-design)
  * [Quick Demo](#quick-demo)
  * [Examples](#examples)
* [Features & Customizability](#features--customizability)
  * [Stringification](#stringification)
  * [Matchers](#matchers)
  * [Policies](#policies)
  * [Bring your own string- and char-types](#bring-your-own-string--and-char-types)
  * [Call-Conventions](#call-conventions)
* [Integration](#integration)
  * [Portability](#portability)
  * [CMake](#cmake)
  * [Packaging Tools](#packaging-tools)
  * [Single-Header](#single-header)
  * [Test Framework](#test-framework)
  * [Documentation](#documentation)
* [Testing](#testing)
* [Known Issues](#known-issues)
* [Special Acknowledgement](#special-acknowledgement)

---

## Introduction

``mimic++`` is a C++20 mocking framework designed to offer a natural and expressive syntax.
While many similar frameworks aim for this goal, they often rely heavily on an extensive list of macros.
Although macros can be beneficial, ``mimic++`` strives to minimize their usage to enhance readability and
maintainability.

What sets ``mimic++`` apart from other mocking frameworks is its unique approach:
Mock objects are explicitly defined as function objects, making them directly callable and overloadable.
This design choice ensures that the framework remains extensible and straightforward to use.

If you're curious to learn more, feel free to explore the [documentation](https://dnkpp.github.io/mimicpp/) and the
cheat-sheet of to the latest release.
You can also check out the examples folder or experiment with the framework online
at [godbolt.org](https://godbolt.org/z/fYavvqfeM).

### Core Design

The framework is built around two core concepts: **Mocks** and **Expectations**.

**Mocks** are objects that simulate functional implementations, allowing you to set up behavior on a per-test-case
basis.
These behaviors are referred to as **Expectations**, which define whether and how the mock should respond during
testing.

The framework diligently tracks all Expectations and reports any that are either violated or left unfulfilled, providing
useful information for debugging.
This tracking mechanism helps developers quickly identify issues in their tests, ensuring that the intended behavior is
accurately represented and maintained.

In essence, Mocks and Expectations work hand in hand to facilitate effective testing.

### Quick Demo

A robust mocking framework must effectively notify users of errors by providing all relevant details — without
overwhelming them with excessive verbosity.

Significant effort has been invested in ensuring that error messages, particularly type information, are clear,
well-structured, and easy to interpret.

Consider the following example where a user creates a Mock object without setting up any expectations, and then proceeds
to call it:

```cpp
mimicpp::Mock<void()> mock{}; // Create a mock without setting up any expectations.
mock();                       // Since no expectation was set up, this call is unmatched.
```

The framework will generate the following output:

```
Unmatched Call originated from `path/to/source.cpp`#L1337, `test_function()`
  On Target `Mock<void()>` used Overload `void()`
No Expectations available!

Stacktrace:
#0 `path/to/source.cpp`#L1337, `test_function()`
// ...
```

In another scenario, when an expectation has been defined but the incoming call fails to meet any of the set criteria,
it is crucial to clearly indicate where the mismatch occurred.
For instance, in the example below the framework expects a call with the argument 42, but receives 1337 instead:

```cpp
mimicpp::Mock<void(int, std::optional<int>)> mock{}; // Create a mock that expects `int` and `std::optional<int>` as argument.
// Set up an expectation:
SCOPED_EXP mock.expect_call(42, std::nullopt);       // The mock should be called with `42` and `nullopt`.
mock(1337, std::nullopt);                            // Call the mock with an incorrect argument (`1337` instead of `42`).
```

This will produce the following detailed output:

```
Unmatched Call originated from `path/to/source.cpp`#L42, `test_function2()`
  On Target `Mock<void(int)>` used Overload `void(int)`
  Where:
      arg[0] => int: 1337
      arg[1] => std::optional<int>: nullopt
1 non-matching Expectation(s):
  #1 Expectation defined at `path/to/source.cpp`#L24, `test_function2()`
  Due to Violation(s):
    - expect: arg[0] == 42
  With Adherence(s):
    + expect: arg[1] == nullopt

Stacktrace:
#0 `path/to/source.cpp`#L42, `test_function2()`
// ...
```

### Examples

<details>
<summary>Mocks as function objects</summary>

As mentioned earlier, ``mimicpp::Mock`` objects are actual function objects and can be used directly.

```cpp
#include <mimic++/mimic++.hpp>

// It is recommended to pull these sub-namespaces out, as doing so enhances the readability of the expectation setup.
namespace expect = mimicpp::expect;
namespace matches = mimicpp::matches;
namespace finally = mimicpp::finally;
namespace then = mimicpp::then;

using matches::_;   // That's the wildcard matcher, which matches anything.

TEST_CASE("Mocks are function objects.")
{
    mimicpp::Mock<int(std::string, std::optional<int>)> mock{};     // This enables the `int operator ()(std::string, std::optional<int>)` member.
    SCOPED_EXP mock.expect_call("Hello, World", _)                  // This requires the first argument to match the string "Hello, World," while the second argument has no restrictions.
                and expect::at_least(1)                             // This specifies that the expectation must be matched one or more times.
                and expect::arg<0>(!matches::range::is_empty())     // Additionally, it requires the first argument to be non-empty (note the preceding !),
                and expect::arg<1>(matches::ne(std::nullopt))       // This requires that the second argument is not std::nullopt...
                and expect::arg<1>(matches::lt(1337))               // ... and is less than 1337.
                and then::apply_arg<0>(                             // That's a side effect that gets executed when a match is made.
                    [](std::string_view str) { std::cout << str; }) //     This one writes the content of the first argument to std::cout.
                and finally::returns(42);                           // It eventually returns 42 for each match.

    int result = mock("Hello, World", 1336);                        // This matches the expectation.
    REQUIRE(42 == result);
}
```

``mimicpp::Mock`` also supports arbitrary overload-sets, enabling an ``operator()`` for each specified signature.

```cpp
TEST_CASE("Mocks can be overloaded.")
{
    mimicpp::Mock<
        int(std::string, std::optional<int>),                   // This uses the same signature as the previous test.
        void() const                                            // Additionally, it enables void operator()() const (note the const specification).
    > mock{};

    SCOPED_EXP mock.expect_call()                               // This sets up an expectation for the void() overload...
                and expect::twice();                            // ... which must be matched twice.

    mock();                                                     // This matches the expectation once.

    // You can create new expectations as needed, even if the mock object is already in use.
    SCOPED_EXP mock.expect_call(!matches::range::is_empty(), 42)        // You can always apply matchers directly; if only a value is provided, it defaults to matches::eq.
                and expect::once()                                      // once() is the default, but you can state that explicitly if desired.
                and finally::throws(std::runtime_error{"some error"});  // When the expectation matches, it will ultimately throw an exception.

    REQUIRE_THROWS(mock("Test", 42));                           // This matches the second expectation and throws the exception as expected.

    // There is still a pending expectation for the void() overload.
    std::as_const(mock)();                                      // This explicitly calls the operator() from a const object.
}
```

</details>

<details>
<summary>Mocks as member functions</summary>

``mimicpp::Mock`` objects can also be used as member functions.
However, this approach has its limitations; for example, they cannot be used via a member function pointer.

```cpp
// Let's build a function that expects an object and requires a .get() member function.
// This member function should return a value that is printable.
inline void foo(const auto& obj)
{
    std::cout << obj.get();
}

TEST_CASE("Mocks can be used as member functions.")
{
    struct Mock
    {
        mimicpp::Mock<int() const> get{}; // This serves as the .get() member function.
    };

    Mock mock{};
    SCOPED_EXP mock.get.expect_call()
                and finally::returns(42);

    foo(mock);                            // The foo-function then calls the get() member function.
}
```

</details>

<details>
<summary>Mocking interfaces</summary>

``mimic++`` also provides utilities for mocking interfaces.

```cpp
// Let's say we have the following interface:
class Interface
{
public:
    virtual ~Interface() = default;
    virtual int get() const = 0;
};

// And a function that actually requires an interface this time.
inline void foo(const Interface& obj)
{
    std::cout << obj.get();
}

TEST_CASE("Interfaces can be mocked.")
{
    class Derived
        : public Interface
    {
    public:
        ~Derived() override = default;

        // This generates the override method and a mock object named `get_`.
        MAKE_MEMBER_MOCK(get, int, (), const override);
    };

    Derived mock{};
    SCOPED_EXP mock.get_.expect_call()      // Note the `_` suffix; that's the name of the mock object.
                and finally::returns(42);

    foo(mock);        // `foo` calls the `get()` member function, which forwards the call to the mock object `get_`.
}
```

Sometimes, an interface method may also have several overloads.
``mimic++`` directly supports overriding overload-sets.

```cpp
// Let's say we have the following interface with an overload set:
class Interface
{
public:
    virtual ~Interface() = default;
    virtual int& get() = 0;
    virtual const int& get() const = 0;
};

// And a function that uses the const overload of that interface.
inline void foo(const Interface& obj)
{
    std::cout << obj.get();
}

TEST_CASE("Interface overload-sets are directly supported.")
{
    class Derived
        : public Interface
    {
    public:
        ~Derived() override = default;

        // This generates two overloads of `get` and a single mock object named `get_`.
        MAKE_OVERLOADED_MEMBER_MOCK(
            get,                                    // The name of the overload set.
            ADD_OVERLOAD(int&, ()),                 // This enables `int& operator ()()`...
            ADD_OVERLOAD(const int&, (), const));   // ... and this the `const int& operator ()() const` on the mock.
    };

    Derived mock{};
    SCOPED_EXP std::as_const(mock).get_.expect_call()   // Since we expect the const overload to be used, we must explicitly select that overload.
                and finally::returns(42);               // The returned reference is valid as long as the expectation is alive.

    foo(mock);                      // `foo` calls the `get() const` member function, which forwards the call to the mock object `get_`, as before.
}
```

</details>

<details>
<summary>Watching object-instances</summary>

The ``mimicpp::Watched`` helper can report the destruction and relocation of object instances.

```cpp
TEST_CASE("LifetimeWatcher and RelocationWatcher can trace the lifetime and relocation of object instances.")
{
    mimicpp::Watched<
        mimicpp::Mock<void()>,
        mimicpp::LifetimeWatcher,
        mimicpp::RelocationWatcher> watched{};

    SCOPED_EXP watched.expect_destruct();
    int relocationCounter{};
    SCOPED_EXP watched.expect_relocate()
                and then::invoke([&] { ++relocationCounter; })
                and expect::at_least(1);

    std::optional wrapped{std::move(watched)};  // This satisfies one relocate-expectation.
    std::optional other{std::move(wrapped)};    // This satisfies a second relocate-expectation.
    
    // This doesn't require a destruct expectation, as moved-from objects are considered dead.
    wrapped.reset();
    
    other.reset();                              // This fulfills the destruct-expectation.
    
    REQUIRE(2 == relocationCounter);            // Let's see how often the instance has been relocated.
}
```

</details>

---

## Features & Customizability

A framework should be a versatile tool that can be utilized in various ways and tailored to meet specific needs.
For this reason, ``mimic++`` offers a range of customization options.
For example, users can create their own expectation policies and integrate them seamlessly without modifying any line of
the ``mimic++`` codebase.

### Matchers

Matchers are used to check whether arguments satisfy specific requirements. While there are many existing matchers
available, users often have unique needs.

``mimic++`` provides a very generic ``mimicpp::PredicateMatcher``, which is often sufficient for most cases.
However, if you need full control, you can start with a fresh type (without any inheritance) and build your own.
Custom matchers simply need to conform to the ``mimicpp::matcher_for`` concept.
For more information, please refer to the documentation.

### Policies

There are multiple types of policies, depending on the tasks they are designed to fulfill.
The **expectation policy** has full control over whether a match can be made or should be rejected,
while the **finalize policy** determines what a mock should do when it actually matches (such as returning a value or
throwing an exception).

These policies can implement arbitrary logic, so feel free to experiment.
There is no base type requirement; they simply need to satisfy either the ``mimicpp::expectation_policy_for``,
``mimicpp::control_policy``, or ``mimicpp::finalize_policy_for``.

### Stringification

``mimic++`` cannot provide stringification for every type, but having a proper textual representation of an object can
be very useful when a test fails.
``mimic++`` will use ``std::format`` (or [fmt](https://github.com/fmtlib/fmt)) for types that are formattable, but
sometimes that may not meet users' needs,
as they might prefer an alternative stringification specifically for testing purposes.

To address this, users can add their own specializations of the ``mimicpp::custom::Printer`` type, allowing them to
specify how a given type should be printed.
Custom specializations will always take precedence over any pre-existing printing methods, enabling users to override
the stringification of internal report types as well.

### Bring your own string- and char-types

If you are working with a large framework, there’s a good chance that it utilizes a custom string or character type (
such as ``QChar`` and ``QString`` from Qt).
While they may appear different, they are essentially just strings, so it would be beneficial to make them fully
compatible with the existing string matchers.

``mimic++`` supports this; users simply need to provide some trait-specializations.
For more information, please refer to the string section of the documentation.

### Call-Conventions

Call conventions are a somewhat controversial topic, as the C++ language definition does not explicitly address them.
However, frameworks like Microsoft's COM utilize the ``__stdcall`` call convention, indicating that at least some
compilers support these specifications.
Consequently, users need the ability to leverage these features.

Since call conventions are not universally portable, ``mimic++`` does not define any conventions itself.
Instead, it provides an easy macro tool, ``MIMICPP_REGISTER_CALL_CONVENTION``, which users can utilize to make the
framework compatible with any call convention they require.

---

## Integration

``mimic++`` is a header-only library, allowing users to easily access all features by simply including the
``mimic++/mimic++.hpp`` header.

### Portability

``mimic++`` is designed to work with any C++20 conforming compiler, independent of the underlying platform or
architecture.
This is achieved by consistently adhering to the language standards,
which is continuously verified through an extensive CI/CD workflow that tracks numerous configurations.

In fact, ``mimic++`` is known to work on Windows, Ubuntu, and macOS with both ``x86_64`` and ``x86_32`` architectures.
For a more comprehensive overview, please refer to the [Testing](#testing) section.

### CMake

The integration into a cmake project is straight-forward.

```cmake
target_link_libraries(
    <your_target_name>
    PUBLIC
    mimicpp::mimicpp
)
```

Users can either select a commit in the **main** branch or a version tag and utilize the CMake ``FetchContent`` module:

```cmake
include(FetchContent)

FetchContent_Declare(
    mimicpp
        VERSION 5 # or GIT_TAG <commit_hash> 
    GIT_REPOSITORY https://github.com/DNKpp/mimicpp
)

FetchContent_MakeAvailable(mimicpp)
# do not forget linking via target_link_libraries as shown above
```

As an alternative, I recommend using [CPM](https://github.com/cpm-cmake/CPM.cmake), which is a convenient wrapper based
on the ``FetchContent`` feature:

```cmake
include(CPM.cmake) # or include(get_cpm.cmake)

CPMAddPackage("gh:DNKpp/mimicpp@5") # or gh:DNKpp/mimicpp#<commit_hash>
# do not forget linking via target_link_libraries as shown above
```

### Packaging Tools

* [vcpkg](https://github.com/Microsoft/vcpkg) - The Microsoft VC++ Packaging Tool.
  Thanks to contributions from community members, ``mimic++`` also has a **vcpkg port**, which can be found
  [here](https://vcpkg.link/ports/mimicpp).

### Single-Header

As an alternative, each release includes a header file named ``mimic++-amalgamated.hpp``, which contains all
definitions (except for the specific test framework adapters)
and can be easily dropped into any C++20 project.
After that, users can simply select the appropriate adapter header from the ``adapters``-folder and include it in their
project as well.

### Test Framework

Mocking frameworks typically do not exist in isolation; rather, they are advanced techniques for creating tests.
They should work seamlessly with any existing test framework.
Therefore, ``mimic++`` provides the ``IReporter`` interface, which serves as a bridge from ``mimic++`` to the utilized
test framework.

``mimic++`` already brings some existing reporter implementations for well-known test frameworks,
but users can also create custom adapters for any test framework or simply use the default reporter.
For more details, please refer to the reporting section in the documentation.

The following official adapters exist and can be included from the `mimic++_ext/adapters` include directory:

* [Boost.Test](https://github.com/boostorg/test) (tested with v1.89.0)
* [Catch2](https://github.com/catchorg) (tested with v3.10.0)
* [Doctest](https://github.com/doctest/doctest) (tested with v2.4.12)
* [GTest](https://github.com/google/googletest) (tested with v1.15.2)

### Documentation

The documentation is generated using Doxygen.
Users can generate it locally by enabling both the ``MIMICPP_CONFIGURE_DOXYGEN`` and ``MIMICPP_ENABLE_GENERATE_DOCS``
CMake options,
and then manually building the target ``mimicpp-generate-docs``.

The documentation for the **main** branch is always available on GitHub Pages.
For the **development** branch, it is also available on the **dev-gh-pages** branch, but unfortunately, it is not
directly viewable in the browser.

Each release includes the generated documentation as an attachment.

---

## Testing

``mimic++`` employs a strict testing policy, ensuring that each official feature is thoroughly tested.
The results of these test cases are consistently tracked by an extensive CI system, which checks compilation success,
test case outcomes,
and coverage across dozens of different operating systems, compilers, and build configurations.

For the test builds, the flags `-Wall -Wextra -Wpedantic -Werror` (or `/W4 /WX /permissive-` on MSVC) are set.
This ensures that `mimic++` won't flood your build output with endless warnings - or, even worse, break your builds —
if you enable these flags in your own projects.

The coverage is generated via `gcov` and evaluated by
[codacy](https://app.codacy.com/gh/DNKpp/mimicpp),
[codecov](https://codecov.io/gh/DNKpp/mimicpp) and
[coveralls](https://coveralls.io/github/DNKpp/mimicpp).

Nevertheless, even with significant effort, achieving 100% code coverage remains out of reach.
I strive to cover each branch, but the coverage percentage can vary between different tools.
The goal is to get as close to 100% as possible.

On the other hand, there is a significant amount of code that isn't even analyzed by these tools, such as templates and
macros.
`mimic++` contains a lot of templated code at its core, which requires at least an equal amount of effort to get right
and thoroughly tested.
Therefore, it's important to take the coverage percentage with a grain of salt.

<details>
<summary>CI Tests</summary>

The listed configurations are explicitly tested, but other do probably work, too.
As new compilers become available, they will be added to the workflow, but older compilers will probably never be
supported.

Note: Unless specified otherwise, it is assumed that each major compiler version refers to its most recent minor
version.

| Symbol |    Description     |
|:------:|:------------------:|
|   x    |       works        |
|   *    | works with caveats |
|   -    |   does not work    |
|   ?    |     not tested     |

**Windows**

| OS           | Compiler | x86_32 | x86_64 | c++-20 | c++-23 | formatting |     stacktrace     |
|--------------|:--------:|:------:|:------:|:------:|:------:|:----------:|:------------------:|
| Windows 2022 |   msvc   |   x    |   x    |   x    |   x    |  std/fmt   | std/cpptrace/boost |
| Windows 2022 | clangCl  |   x    |   x    |   x    |   x    |  std/fmt   | std/cpptrace/boost |

**Linux**

| Compiler | x86_32 | x86_64 | libstdc++ | libc++ | c++-20 | c++-23 | formatting |     stacktrace      |
|----------|:------:|:------:|:---------:|:------:|:------:|:------:|:----------:|:-------------------:|
| clang-16 |   x    |   x    |     x     |   x    |   x    |   x    |    fmt     |   cpptrace/boost    |
| clang-17 |   x    |   x    |     x     |   x    |   x    |   x    |  std/fmt   | std*/cpptrace/boost |
| clang-18 |   x    |   x    |     x     |   *    |   x    |   x    |  std/fmt   | std*/cpptrace/boost |
| clang-19 |   x    |   x    |     x     |   x    |   x    |   x    |  std/fmt   | std*/cpptrace/boost |
| clang-20 |   x    |   x    |     x     |   x    |   x    |   x    |  std/fmt   | std*/cpptrace/boost |
| gcc-10   |   *    |   *    |     *     |   ?    |   *    |   *    |    fmt     |   cpptrace/boost    |
| gcc-10.2 |   *    |   *    |     *     |   ?    |   *    |   *    |    fmt     |   cpptrace/boost    |
| gcc-11   |   x    |   x    |     x     |   ?    |   x    |   x    |    fmt     |   cpptrace/boost    |
| gcc-12   |   x    |   x    |     x     |   ?    |   x    |   x    |    fmt     |   cpptrace/boost    |
| gcc-13   |   x    |   x    |     x     |   ?    |   x    |   x    |  std/fmt   | std*/cpptrace/boost |
| gcc-14   |   x    |   x    |     x     |   ?    |   x    |   x    |  std/fmt   | std*/cpptrace/boost |
| gcc-15   |   x    |   x    |     x     |   ?    |   x    |   x    |  std/fmt   | std*/cpptrace/boost |

Note:

- `libc++` doesn't support `std::stacktrace` yet.
- `gcc-10` doesn't support `std::source_location`, but `mimic++` falls back to compiler intrinsics in these cases.
- `gcc-10.2` is the latest version on Debian-11 (aka `bullseye`) and thus explicitly listed here.

**macOS**

| Compiler          | x86_64 | libstdc++ | libc++ | c++-20 | c++-23 | formatting |     stacktrace      |
|-------------------|:------:|:---------:|:------:|:------:|:------:|:----------:|:-------------------:|
| AppleClang-16.0.6 |   x    |     ?     |   x    |   x    |   x    |    fmt     |   cpptrace/boost    |
| AppleClang-17.0.6 |   x    |     ?     |   x    |   x    |   x    |  std/fmt   | std*/cpptrace/boost |
| AppleClang-18.1.6 |   x    |     ?     |   x    |   x    |   x    |  std/fmt   | std*/cpptrace/boost |

Note: macOS officially doesn't support 32bit builds, so they are not tested.

</details>

---

## Known Issues

### Clang-18.1 + libc++

Date: 25.09.2024

This combination introduced a regression regarding the ``std::invocable`` concept and default parameter(s).
On this version, all invocable checks will fail, but the ``std::is_invocable`` trait still works as expected.
Unfortunately this can not solved easily by this framework - sorry for that.

Clang-17 and Clang-19 do not suffer from this issue.
For more information have a look [here](https://github.com/llvm/llvm-project/issues/106428).

---

## Special Acknowledgement

This framework is heavily inspired by the well-known [trompeloeil](https://github.com/rollbear/trompeloeil), which I
have personally used for several years.
While it is definitely a solid choice, it can sometimes feel a bit dated,
and some macros may not work well with formatting tools and similar utilities.
If you need a pre-C++20 mocking framework, I highly recommend giving it a try.

Fun fact: ``mimic++`` uses ``trompeloeil`` for its own test suite :D
