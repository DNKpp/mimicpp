#          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

target_sources(${TARGET_NAME} PRIVATE
    "Common.cpp"
    "FloatingPointMatchers.cpp"
    "GeneralMatchers.cpp"
    "RangeMatchers.cpp"
    "StringContains.cpp"
    "StringEndsWith.cpp"
    "StringEq.cpp"
    "StringStartsWith.cpp"
)
