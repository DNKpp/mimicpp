#          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

set(UNI_ALGO_INSTALL ON CACHE BOOL "")

message(DEBUG "${MESSAGE_PREFIX} Searching for `uni-algo`-package.")
find_package(uni-algo QUIET)
if (NOT uni-algo_FOUND)
    include(get_cpm)
    CPMAddPackage("gh:uni-algo/uni-algo@1.2.0")
endif ()
