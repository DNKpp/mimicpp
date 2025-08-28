#          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

if (NOT TARGET mimicpp::internal::enable-std-stacktrace)
    add_library(mimicpp-internal-enable-std-stacktrace INTERFACE)
    add_library(mimicpp::internal::enable-std-stacktrace ALIAS mimicpp-internal-enable-std-stacktrace)

    target_compile_features(mimicpp-internal-enable-std-stacktrace INTERFACE
        cxx_std_23
    )
    target_compile_definitions(mimicpp-internal-enable-std-stacktrace INTERFACE
        MIMICPP_CONFIG_EXPERIMENTAL_STACKTRACE=1
        MIMICPP_CONFIG_EXPERIMENTAL_USE_CXX23_STACKTRACE=1
    )
endif ()

if (NOT TARGET mimicpp::internal::enable-cpptrace)
    add_library(mimicpp-internal-enable-cpptrace INTERFACE)
    add_library(mimicpp::internal::enable-cpptrace ALIAS mimicpp-internal-enable-cpptrace)

    target_compile_definitions(mimicpp-internal-enable-cpptrace INTERFACE
        MIMICPP_CONFIG_EXPERIMENTAL_STACKTRACE=1
        MIMICPP_CONFIG_EXPERIMENTAL_USE_CPPTRACE=1
        $<$<BOOL:${MIMICPP_CONFIG_EXPERIMENTAL_IMPORT_CPPTRACE}>:MIMICPP_CONFIG_EXPERIMENTAL_IMPORT_CPPTRACE=1>
    )
endif ()

if (NOT TARGET mimicpp::internal::enable-boost-stacktrace)
    add_library(mimicpp-internal-enable-boost-stacktrace INTERFACE)
    add_library(mimicpp::internal::enable-boost-stacktrace ALIAS mimicpp-internal-enable-boost-stacktrace)

    target_compile_definitions(mimicpp-internal-enable-boost-stacktrace INTERFACE
        MIMICPP_CONFIG_EXPERIMENTAL_STACKTRACE=1
        MIMICPP_CONFIG_EXPERIMENTAL_USE_BOOST_STACKTRACE=1
    )
endif ()

if (NOT TARGET mimicpp::internal::enable-custom-stacktrace)
    add_library(mimicpp-internal-enable-custom-stacktrace INTERFACE)
    add_library(mimicpp::internal::enable-custom-stacktrace ALIAS mimicpp-internal-enable-custom-stacktrace)

    target_compile_definitions(mimicpp-internal-enable-custom-stacktrace INTERFACE
        MIMICPP_CONFIG_EXPERIMENTAL_STACKTRACE=1
        MIMICPP_CONFIG_EXPERIMENTAL_USE_CUSTOM_STACKTRACE=1
    )
endif ()
