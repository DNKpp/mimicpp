#          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

if (NOT MIMICPP_CONFIG_DISABLE_PRETTY_TYPE_PRINTING)

    target_sources(${TARGET_NAME}
        PRIVATE
        "TypePrinter.cpp"
        "ContainerTypePrinting.cpp"
    )

endif ()
