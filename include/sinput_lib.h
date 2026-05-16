/*
 * SINPUT-LIB-HID — Public application hooks and helpers for building 64-byte HID input reports.
 *
 * Copyright (c) 2026 Hand Held Legend, LLC
 * Author: Mitchell Cairns
 *
 * SPDX-License-Identifier: MIT-0
 */

#ifndef SINPUT_LIB_H
#define SINPUT_LIB_H

#include <stdint.h>
#include <stdbool.h>

#include "sinput_lib_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Validates and stores device configuration, then enables the SINPUT pipeline.
 *
 * @param cfg In/out device capability and identity; invalid fields may be overridden.
 * @return Configuration outcome (#sinput_config_status_t).
 */
sinput_config_status_t sinput_api_init(sinput_device_cfg_s *cfg);

/**
 * @brief Host-requested left/right ERM-style rumble (implement in firmware).
 * @note Weak default is no-op; override in your port.
 */
void sinput_api_hook_set_rumble(sinput_stereo_rumble_s rumble);

/**
 * @brief Host-requested HD-style dual-actuator haptics (implement in firmware).
 * @note Weak default is no-op; override in your port.
 */
void sinput_api_hook_set_haptics(sinput_stereo_haptics_s haptics);

/**
 * @brief Sets player index LEDs (0-based or product-specific mapping).
 * @note Weak default is no-op; override in your port.
 */
void sinput_api_hook_set_player_leds(uint8_t player_number);

/**
 * @brief Sets RGB color for joystick rim lighting (32-bit packed RGB, product-defined).
 * @note Weak default is no-op; override in your port.
 */
void sinput_api_hook_set_joystick_rgb(uint32_t rgb_value);

/**
 * @brief Read battery / USB power status for the current input report.
 * @param status Filled when the hook returns true.
 * @return True if @a status was updated.
 * @note Weak default returns false; override in your port.
 */
bool sinput_api_hook_get_power(sinput_power_s *status);

/**
 * @brief Read digital button bitfields for the current input report.
 * @param out Filled when the hook returns true.
 * @return True if @a out was updated.
 * @note Weak default returns false; override in your port.
 */
bool sinput_api_hook_get_buttons(sinput_buttons_s *out);

/**
 * @brief Read left/right analog stick samples (typically INT16 range per axis).
 * @param out Filled when the hook returns true.
 * @return True if @a out was updated.
 * @note Weak default returns false; override in your port.
 */
bool sinput_api_hook_get_joysticks(sinput_joysticks_s *out);

/**
 * @brief Read left/right analog trigger raw values (scaled internally for the wire format).
 * @param out Filled when the hook returns true.
 * @return True if @a out was updated.
 * @note Weak default returns false; override in your port.
 */
bool sinput_api_hook_get_triggers(sinput_triggers_s *out);

/**
 * @brief Read IMU samples and timestamp for the current input report.
 * @param out Filled when the hook returns true.
 * @return True if @a out was updated.
 * @note Weak default returns false; override in your port.
 */
bool sinput_api_hook_get_motion(sinput_motion_s *out);

/**
 * @brief Read touchpad / trackpad samples (left/right pads).
 * @param out Filled when the hook returns true.
 * @return True if @a out was updated.
 * @note Weak default returns false; override in your port.
 */
bool sinput_api_hook_get_touchpads(sinput_touchpads_s *out);

/**
 * @brief Fills a 64-byte HID buffer with either the main gamepad report or a feature reply.
 *
 * @param out 64-byte output buffer (report layout is defined by the SINPUT HID report descriptor).
 * @return False only if @a out is NULL or generation failed.
 */
bool sinput_api_generate_inputreport(uint8_t out[64]);

/**
 * @brief Entry point for host-to-device output reports (haptics, RGB, feature requests).
 *
 * Parses vendor output report ID 3; forwarding from your USB/transport stack is required.
 *
 * @param data Raw bytes as received from the host (includes report ID in @a data[0] when applicable).
 * @param len  Length of @a data in bytes.
 */
void sinput_api_output_tunnel(const uint8_t *data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif
