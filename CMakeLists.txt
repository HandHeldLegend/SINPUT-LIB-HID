# SINPUT-LIB-HID - USB HID input report builder and descriptor data for embedded gamepads.
# Copyright (c) 2026 Hand Held Legend, LLC. Author: Mitchell Cairns. SPDX-License-Identifier: MIT-0
#
# Portable C: no SDK or chip-specific link dependencies. Link your toolchain's C library when you
# build the final executable or wrapper library.

add_library(sinput_lib_hid STATIC
    ${CMAKE_CURRENT_LIST_DIR}/sinput_lib.c
    ${CMAKE_CURRENT_LIST_DIR}/sinput_lib_config.c
    ${CMAKE_CURRENT_LIST_DIR}/sinput_lib_hid.c
    ${CMAKE_CURRENT_LIST_DIR}/sinput_lib_protocol.c
)

target_include_directories(sinput_lib_hid PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

set_target_properties(sinput_lib_hid PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
)
