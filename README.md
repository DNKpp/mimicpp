# mimic++, a modern and (mostly) macro free mocking framework

[![codecov](https://codecov.io/gh/DNKpp/mimicpp/graph/badge.svg?token=T9EpgyuyUi)](https://codecov.io/gh/DNKpp/mimicpp)
[![Coverage Status](https://coveralls.io/repos/github/DNKpp/mimicpp/badge.svg)](https://coveralls.io/github/DNKpp/mimicpp)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/b852271c6e8742fe8a1667e679dc422b)](https://app.codacy.com/gh/DNKpp/mimicpp/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
[![Codacy Badge](https://app.codacy.com/project/badge/Coverage/b852271c6e8742fe8a1667e679dc422b)](https://app.codacy.com/gh/DNKpp/mimicpp/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_coverage)
[![Documentation](https://img.shields.io/badge/docs-doxygen-blue)](https://dnkpp.github.io/mimicpp/)

---

## Author
Dominic Koepke  
Mail: [DNKpp2011@gmail.com](mailto:dnkpp2011@gmail.com)

## License
[BSL-1.0](LICENSE_1_0.txt) (free, open source)

```text
          Copyright Dominic "DNKpp" Koepke 2024 - 2024
 Distributed under the Boost Software License, Version 1.0.
    (See accompanying file LICENSE_1_0.txt or copy at
          https://www.boost.org/LICENSE_1_0.txt)
```

---

## Introduction
``mimic++`` is a c++20 mocking framework, which aims to offer a very natural end expressive syntax, without constantly resorting to macros.
To be honest, macros cannot be completely avoided, but they can at least be reduced to a very minimum.

The one thing, that this framework does different than all other (or at least all I am aware of) mocking framework is, that mock objects explicitly are function objects,
thus directly callable. It's simple and straight forward.

As I'm mainly working on template or functional-style code, I wanted something simpler than always creating explicit mock types for the simplest of use cases.
So, ``mimicpp::Mock`` objects can directly be used as functional objects, but they can also be used as member objects and thus serve as actual member functions.
If you are curious, have a look at the documentation or directly into the examples.

### Design Philosophy
The framework is designed with two core concepts in mind: Mocks and Expectations.
Mocks can be used to define behaviour on a per test-case basis, without the necessity of creating dozens of types. The go-to example is,
if you have a custom type, which somehow makes a connection to a concrete database, you do not want to setup an actual database connection during
your test runs. You then simply install a database mock, which then yields the exact replies as it were defined for that particular case:
the so called "Expectations".

So, Mocks and Expectations are playing together hand in hand.

### Basic Examples
Mocks themselves are very easy to create:
```cpp
mimicpp::Mock<void()> myMock{};
```
This already is a fully functional Mock, which enables a member ``void operator()`` for which Expectations can be created.

```cpp
mimicpp::ScopedExpectation myExpectation = myMock.expect_call();
```
The ``expect_call()`` member function initiates an expectation. Expectations are usually required to be fulfilled within the current (or deeper) scope.
The ``ScopedExpectation`` then takes over the responsibility to check the Expectation, when that scope is left. Usually users do not need direct
access to the expectations but still an unique name is required. To overcome that language limitation, an optional macro can be used:
```cpp
SCOPED_EXP myMock.expect_call();
```
This effectively does the same job as before, but the macro takes over the burden creating an unique name for that expectation.

Given the previously created expectation, it is expected, that the call operator of ``myMock`` is called exactly once:
```cpp
myMock();
```

Expectations can contain several requirements, e.g. ``times`` which indicates, how often an Expectation must be met.
For more examples, have a look into the documentation or directly into the ``examples`` directory.


### Special Acknowledgement
This framework is highly inspired by the well known [trompeloeil](https://github.com/rollbear/trompeloeil), which I have used myself for several years now.
It's definitly not bad, but sometimes feels a little bit dated and some macros do not play very well with formatting tools and the like.
If you need a pre-c++20 mocking-framework, you should definitly give it a try.

Fun fact: ``mimic++`` uses ``trompeloeil`` for it's own test suite :D

## Documentation
The documenation is generated via ``doxygen``. Users can do this locally by enabling both, the ``MIMICPP_CONFIGURE_DOXYGEN`` and ``MIMICPP_CONFIGURE_DOXYGEN``,
cmake options and building the target ``mimicpp-generate-docs`` manually.

The documentation for the ``main`` branch is always available on the github pages; for the ``development`` branch it is also available on the ``dev-gh-pages`` branch,
but unfortunatly not directly viewable on the browser.
Every release has the generated documentation attached.

## Installation
This framework is header-only and completely powered by cmake, thus the integration into a cmake project is straight-forward.
```cmake
target_link_libraries(
	<your_target_name>
	PUBLIC
	mimicpp::mimicpp
)
```

Users can either pick a commit in the ``main`` branch or a version tag and utilize the cmake ``FetchContent`` module:
```cmake
include(FetchContent)

FetchContent_Declare(
	mimicpp
	GIT_REPOSITORY	https://github.com/DNKpp/mimicpp
	GIT_TAG		<any_commit_hash_or_tag>
)

FetchContent_MakeAvailable(mimicpp)
# do not forget linking via target_link_libraries as shown above
```

As an alternative, I recommend the usage of [CPM](https://github.com/cpm-cmake/CPM.cmake), which is a featureful wrapper around the ``FetchContent``
functionality:
```cmake
include(CPM.cmake) # or include(get_cpm.cmake)

CPMAddPackage("gh:DNKpp/mimicpp#<any_commit_hash_or_tag>")
# do not forget linking via target_link_libraries as shown above
```

## Testing
``mimic++`` utilizes a strict testing policy, thus each official feature is well tested. The effect of those test-cases are always tracked by the extensive ci,
which checks the compilation success, test cases outcomes and coverage on dozens of different os, compiler and build configurations.

The coverage is generated via ``gcov`` and evaluated by
[codacy](https://app.codacy.com/gh/DNKpp/mimicpp/dashboard),
[codecov](https://app.codecov.io/gh/DNKpp/mimicpp) and
[coveralls](https://coveralls.io/github/DNKpp/mimicpp).

### Windows
| OS           | Compiler | c++-20 | c++-23 |
|--------------|----------|:------:|:------:|
| Windows 2022 | msvc     |    x   |    x   |
| Windows 2022 | clangCl  |    x   |    x   |

### Linux
| Compiler | libstdc++ | libc++ | c++-20 | c++-23 |
|----------|:---------:|:------:|:------:|:------:|
| clang-17 |     x     |    x   |    x   |    x   |
| clang-18 |     x     |    x   |    x   |    x   |
| gcc-13   |     x     |    -   |    x   |    x   |
| gcc-14   |     x     |    -   |    x   |    x   |

### MacOs
| Compiler          | libstdc++ | libc++ | c++-20 | c++-23 |
|-------------------|:---------:|:------:|:------:|:------:|
| AppleClang-17.0.6 |     -     |    x   |    x   |    x   |

As new compilers become available, they will be added to the workflow, but older compilers will probably never be supported.
