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
 * \anchor MIMICPP_CONFIG_ONLY_PREFIXED_MACROS
 * ## Disable unprefixed macros
 * Name: ``MIMICPP_CONFIG_ONLY_PREFIXED_MACROS``
 * 
 * By default, ``mimic++`` defines several shorthand macros for their longer counterparts, to make it less verbose (e.g \ref SCOPED_EXP is shorthand for
 * \ref MIMICPP_SCOPED_EXPECTATION). Even if very unlikely, this may lead to name clashes with other dependencies. Because of that, the option
 * \ref MIMICPP_CONFIG_ONLY_PREFIXED_MACROS is offered, which then disables all shorthand macros.
 * 
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
 */
