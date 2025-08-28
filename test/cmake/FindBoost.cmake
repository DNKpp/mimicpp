#          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

include(get_cpm)

set(BOOST_INCLUDE_LIBRARIES test stacktrace)
CPMAddPackage(
    NAME Boost
    URL "https://github.com/boostorg/boost/releases/download/boost-1.89.0/boost-1.89.0-cmake.tar.gz"
    URL_HASH "SHA256=954a01219bf818c7fb850fa610c2c8c71a4fa28fa32a1900056bcb6ff58cf908"
    VERSION 1.89.0
    EXCLUDE_FROM_ALL YES
    SYSTEM YES
)
