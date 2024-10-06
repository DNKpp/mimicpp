// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

/**
 * \defgroup CONFIGURATION framework configuration
 * \brief Contains documentation for the possible configuration options.
 * \details ``mimic++`` offers various configuration options, which are listed below.
 * In general, these options can be enabled by defining a macro with that specified name, before any ``mimic++`` header is included.
 * If ``mimic++`` is integrated as cmake-target, it also offers appropriate cmake-options with the same name,
 * which can be set during cmake configuration. If an option is enabled, ``mimic++`` defines the relevant macro by itself.
 * 
 * ---
 * \anchor MIMICPP_CONFIG_ONLY_PREFIXED_MACROS
 * ## Disable unprefixed macros
 * Name: ``MIMICPP_CONFIG_ONLY_PREFIXED_MACROS``
 * 
 * By default, ``mimic++`` defines several shorthand macros for their longer counterparts, to make it less verbose (e.g \ref SCOPED_EXP is shorthand for
 * \ref MIMICPP_SCOPED_EXPECTATION). Even if very unlikely, this may lead to name clashes with other dependencies. Because of that, the option
 * \ref MIMICPP_CONFIG_ONLY_PREFIXED_MACROS is offered, which then disables all shorthand macros.
 * 
 * ---
 * \anchor MIMICPP_CONFIG_USE_FMT
 * ## Use ``fmt`` as formatting backend
 * Name: ``MIMICPP_CONFIG_USE_FMT``
 * \see https://github.com/fmtlib/fmt
 * 
 * As of c++20, the format library is offically part of the ``std``. Unfortunatly some compilers, which already support the majority of c++20 features,
 * do not support the formatting part. In such cases, users can enable this option and thus use ``fmt`` instead.
 * 
 * Whatever the reason to use ``fmt`` might be, chances are good, that the library is already present. ``mimic++`` tries to detect that when linked
 * as cmake-target, before pulling it from github. If you want to reuse an existing ``fmt`` package, make sure, that it can be found via ``find_package(fmt)``.
 * 
 * ---
 * \anchor MIMICPP_CONFIG_EXPERIMENTAL_CATCH2_MATCHER_INTEGRATION
 * ## Enable experimental catch2-matcher integration
 * Name: ``MIMICPP_CONFIG_EXPERIMENTAL_CATCH2_MATCHER_INTEGRATION``
 * 
 * If enabled, all matchers from the ``Catch::Matcher`` namespaces can be directly used everywhere, where ``mimic++``-matchers are suitable.
 * \attention This is an experimental feature, and may be removed during any release.
 * 
 * ### Why is it an experimental feature?
 * Unfortunatly ``catch2`` matchers are desgined to be used in-place. Every combination you do, like negation via ``operator !`` or ``operator &&``,
 * just takes a const-reference to the source-matcher.
 * \see https://github.com/catchorg/Catch2/blob/devel/docs/matchers.md#combining-operators-and-lifetimes
 * 
 * ``mimic++`` needs to store the matcher inside the expectation. These requirements are not playing well together.
 * Nevertheless ``catch2`` offers some nice matcher definitions, which could be very helpfull for testing. But as already said, they are dangerous,
 * thus I decided to make them opt-in as experimental features.
 * 
 * ---
 * \anchor MIMICPP_CONFIG_EXPERIMENTAL_UNICODE_STR_MATCHER
 * ## Enable experimental string-matcher unicode support
 * Name: ``MIMICPP_CONFIG_EXPERIMENTAL_UNICODE_STR_MATCHER``
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
 * ### Why is it an experimental feature?
 * 
 * I recently switched from ``cpp-unicodelib``, which I didn't like very much for several reasons. ``uni-algo`` seems more mature, but I would like
 * to get some feedback, before I'll declare this as a stable feature.
 */
