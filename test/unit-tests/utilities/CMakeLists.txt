#          Copyright Dominic (DNKpp) Koepke 2024-2026.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

target_sources(${TARGET_NAME} PRIVATE
    "Algorithm.cpp"
    "Concepts.cpp"
    "CopyableBox.cpp"
    "C++23Backports.cpp"
    "SourceLocation.cpp"
    "Stacktrace.cpp"
    "StaticString.cpp"
    "TypeList.cpp"
)
