/*
 * SINPUT-LIB-HID — Protocol helpers (reports, scaling, feature payload, parsing).
 *
 * Copyright (c) 2026 Hand Held Legend, LLC
 * Author: Mitchell Cairns
 *
 * SPDX-License-Identifier: MIT-0
 */

#ifndef SINPUT_LIB_PROTOCOL_H
#define SINPUT_LIB_PROTOCOL_H

#include "sinput_lib_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Builds a 64-byte input or feature reply report using current hooks and configuration.
 * @param out HID payload buffer (64 bytes); first byte is report ID for USB framing upstream.
 * @return False only when @a out is null; otherwise true (including feature generation path).
 */
bool sinput_protocol_generate_inputreport(uint8_t out[64]);

/**
 * @brief Handles host output reports (haptics, LED, joystick RGB, feature request handshake).
 * @param data Raw report bytes; must include leading report ID @c 0x03 for vendor output path.
 * @param len Number of bytes in @a data; values under 2 are ignored.
 */
void sinput_protocol_output_tunnel(const uint8_t *data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* SINPUT_LIB_PROTOCOL_H */
