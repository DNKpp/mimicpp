//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

/**
 * \defgroup CONFIGURATION framework configuration
 * \brief Contains documentation for the possible configuration options.
 * \details ``mimic++`` offers various configuration options, which are listed below.
 * In general, these options can be enabled by defining a macro with that specified name, before any ``mimic++`` header is included.
 * If ``mimic++`` is integrated as cmake-target, it also offers appropriate cmake-options with the same name,
 * which can be set during cmake configuration. If an option is enabled, ``mimic++`` defines the relevant macro by itself.
 *
 * ---
 *
 * \anchor MIMICPP_CONFIG_ALTERNATIVE_SOURCE_LOCATION_BACKEND
 * ## Install an alternative source-location backend.
 * **Name:** ``MIMICPP_CONFIG_ALTERNATIVE_SOURCE_LOCATION_BACKEND``
 *
 * By default, `mimic++` uses the C++20 `std::source_location`. However, this feature is not available on all platforms or with all compilers.
 * To support older environments, users can configure an alternative by setting this variable to a fully qualified type name.
 * \note The provided type must satisfy the requirements of the `mimicpp::util::source_location::backend` concept.
 * Refer to the \ref SOURCE_LOCATION "source-location" section for more details.
 *
 * ---
 *
 * \anchor MIMICPP_CONFIG_ONLY_PREFIXED_MACROS
 * ## Disable unprefixed macros
 * **Name:** ``MIMICPP_CONFIG_ONLY_PREFIXED_MACROS``
 * 
 * By default, ``mimic++`` defines several shorthand macros for their longer counterparts, to make it less verbose (e.g \ref SCOPED_EXP is shorthand for
 * \ref MIMICPP_SCOPED_EXPECTATION). Even if very unlikely, this may lead to name clashes with other dependencies. Because of that, the option
 * \ref MIMICPP_CONFIG_ONLY_PREFIXED_MACROS is offered, which then disables all shorthand macros.
 *
 * ---
 *
 * \anchor MIMICPP_CONFIG_USE_FMT
 * ## Use `fmt` as formatting backend
 * **Name:** `MIMICPP_CONFIG_USE_FMT`
 * \see https://github.com/fmtlib/fmt
 * 
 * As of c++20, the format library is officially part of the `std`.
 * Unfortunately some compilers — while supporting most C++20 features — do not yet support the formatting library.
 * In such cases, users can enable this option to use `fmt` as a fallback.
 * 
 * Whatever the reason to use `fmt` might be, chances are good that the library is already present.
 * `mimic++` attempts to detect it when linked as CMake target, before pulling it from GitHub.
 * To reuse an existing `fmt` package, ensure it can be found via `find_package(fmt)`.
 *
 * If C++20 modules are supported, you can also control how `fmt` is included via the `MIMICPP_CONFIG_IMPORT_FMT` option (default: `ON`).
 * When set to `ON`, `import fmt;` will be used.
 * Otherwise, it will fall back to a traditional `#include` directive.
 *
 * ---
 *
 * \anchor MIMICPP_CONFIG_EXPERIMENTAL_CATCH2_MATCHER_INTEGRATION
 * ## Enable experimental catch2-matcher integration
 * **Name:** ``MIMICPP_CONFIG_EXPERIMENTAL_CATCH2_MATCHER_INTEGRATION``
 * 
 * If enabled, all matchers from the ``Catch::Matcher`` namespaces can be directly used everywhere, where ``mimic++``-matchers are suitable.
 * \attention This is an experimental feature, and may be removed during any release.
 * 
 * ### Why is this feature experimental?
 *
 * Unfortunatly ``catch2`` matchers are desgined to be used in-place. Every combination you do, like negation via ``operator !`` or ``operator &&``,
 * just takes a const-reference to the source-matcher.
 * \see https://github.com/catchorg/Catch2/blob/devel/docs/matchers.md#combining-operators-and-lifetimes
 * 
 * ``mimic++`` needs to store the matcher inside the expectation. These requirements are not playing well together.
 * Nevertheless ``catch2`` offers some nice matcher definitions, which could be very helpfull for testing. But as already said, they are dangerous,
 * thus I decided to make them opt-in as experimental features.
 *
 * ---
 *
 * \anchor MIMICPP_CONFIG_EXPERIMENTAL_UNICODE_STR_MATCHER
 * ## Enable experimental string-matcher unicode support
 * **Name:** ``MIMICPP_CONFIG_EXPERIMENTAL_UNICODE_STR_MATCHER``
 * 
 * If enabled, all string-matchers get full unicode support. This is relevant, when comparing case-insensitively. It's not required for the base-version
 * of string-matchers.
 * 
 * This will require the rather light-weight library ``uni-algo``, which can be found on github. 
 * \see https://github.com/uni-algo/uni-algo
 * 
 * ``mimic++`` first tries to find the dependency via ``find_package``, but will fetch it from github if not available.
 * 
 * \attention This is an experimental feature, and may be removed during any release.
 * 
 * ### Why is this feature experimental?
 * 
 * I recently switched from ``cpp-unicodelib``, which I didn't like very much for several reasons. ``uni-algo`` seems more mature, but I would like
 * to get some feedback, before I'll declare this as a stable feature.
 *
 * ---
 *
 * \anchor MIMICPP_CONFIG_EXPERIMENTAL_STACKTRACE
 * ## Enable experimental stacktrace support
 * **Name:** ``MIMICPP_CONFIG_EXPERIMENTAL_STACKTRACE``
 *
 * When enabled, ``mimic++`` uses stacktrace information to provide more useful information to the users.
 * Users can decide whether they use the c++23 ``std::stacktrace``, the third-party ``cpptrace`` or any custom stacktrace-backend.
 * \see \ref STACKTRACE "stacktrace" documentation
 * \see \ref MIMICPP_CONFIG_EXPERIMENTAL_USE_CPPTRACE
 *
 * \attention This is an experimental feature, which may be removed during any release.
 *
 * ### Why is this feature experimental?
 *
 * Stack traces require significantly more work for each mock call. It can be worthwhile, but it can also be too excessive.
 *
 * ---
 *
 * \anchor MIMICPP_CONFIG_EXPERIMENTAL_USE_CPPTRACE
 * ## Enable experimental cpptrace stacktrace-backend
 * **Name:** `MIMICPP_CONFIG_EXPERIMENTAL_USE_CPPTRACE`
 * \see https://github.com/jeremy-rifkin/cpptrace
 *
 * When enabled, `mimic++` installs `cpptrace` as the default stacktrace-backend.
 * \note This option is only available, if \ref MIMICPP_CONFIG_EXPERIMENTAL_STACKTRACE is enabled.
 *
 * `mimic++` attempts to detect whether `cpptrace` is already available before pulling it from GitHub.
 * If you want to reuse an existing `cpptrace` package, make sure that it can be found via `find_package(cpptrace)`.
 *
 * If C++20 modules are supported, you can also control how `cpptrace` is included via the `MIMICPP_CONFIG_EXPERIMENTAL_IMPORT_CPPTRACE` option (default: `ON`).
 * When set to `ON`, `import cpptrace;` will be used (requires version `1.0.0` or later).
 * Otherwise, it will fall back to a traditional `#include` directive.
 *
 * \attention This is an experimental feature, which may be removed during any release.
 *
 * ### Why is this feature experimental?
 *
 * General stacktrace support in `mimic++` is currently marked as experimental.
 * Since this backend depends on that feature, it is also considered experimental.
 *
 * ---
 *
 * \anchor MIMICPP_CONFIG_EXPERIMENTAL_PRETTY_TYPES
 * ## Enable experimental pretty type-printing
 * **Name:** ``MIMICPP_CONFIG_EXPERIMENTAL_PRETTY_TYPES``
 *
 * When enabled, `mimic++` enhances type printing to closely match their actual representation in code,
 * making error messages **more readable and informative**.
 *
 * However, this feature can introduce additional overhead, especially with heavily templated types.
 * Without it, most types will be printed using `typeid(T).name()`, or, if `mimic++` is given a stringified type-name,
 * it will output that string as-is.
 * This usually provides a **rough indication of the type**, but often includes extra noise.
 *
 * \attention This is an experimental feature, which may be removed during any release.
 *
 * ### Why is this feature experimental?
 *
 * The complexity lies in **post-processing** type strings into a uniform and human-readable format.
 * Since compilers produce **inconsistent and platform-dependent** type names, `mimic++` must handle **a wide range of patterns, quirks, and edge cases**.
 * While great effort has been made to make this reliable, unexpected cases may still exist in certain environments.
 *
 * ---
 *
 * \anchor MIMICPP_CONFIG_EXPERIMENTAL_ENABLE_CXX20_MODULES__UNPORTABLE__
 * ## Enable experimental C++20 modules.
 * **Name:** `MIMICPP_CONFIG_EXPERIMENTAL_ENABLE_CXX20_MODULES__UNPORTABLE__`
 *
 * C++20 modules have come a long way and are finally appearing on the horizon.
 * Unfortunately, they still require a very up-to-date toolchain (CMake, compiler, etc.), which is often not available by default.
 * Even then, there's a significant chance of encountering compiler bugs that are difficult or impossible to work around.
 * Because of this, users must explicitly opt-in to this feature.
 * \note This option is only available when CMake actually supports C++20 Modules (i.e. version `3.28` and later).
 *
 * When enabled, this defines the CMake target `mimicpp::module` and makes `mimicpp::mimicpp` an alias for it.
 * \note Users can still explicitly choose to link against the header-only target via `mimicpp::header-only`.
 *
 * In addition, this option signals that all (optional) dependencies should be consumed as modules when they provide one.
 *
 * \attention This is an experimental and **highly unportable** feature.
 * Its behavior may change in any release.
 * However, it is unlikely to be removed entirely and will probably continue to evolve as C++20 module support stabilizes across toolchains.
 *
 * ---
 *
 */
