/*
 * SINPUT-LIB-HID - Public application hooks and helpers for building 64-byte HID input reports.
 *
 * Platform-agnostic portable C: no dependency on a specific MCU, USB stack, or Bluetooth stack.
 * Works with wired USB and Bluetooth HID transports: call the same APIs from your stack’s IN/OUT
 * or notification paths.
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
 * @brief Validates and stores device configuration (capabilities and parameters) for the SINPUT pipeline.
 *
 * @param cfg In/out device capability fields; invalid fields may be overridden.
 * @return Configuration outcome (#sinput_config_status_t).
 */
sinput_config_status_t sinput_api_init(sinput_device_cfg_s *cfg);

/**
 * @brief Host-requested left/right ERM-style rumble (implement in firmware).
 * @note Weak default is no-op; override in your port.
 * @note Hosts may also use @ref sinput_api_hook_set_haptics. Conforming gamepads handle **both** encodings;
 *       map to your actuators if the hardware only supports one physical mode.
 */
void sinput_api_hook_set_rumble(sinput_stereo_rumble_s rumble);

/**
 * @brief Host-requested HD-style dual-actuator haptics (implement in firmware).
 * @note Weak default is no-op; override in your port.
 * @note Hosts may also use @ref sinput_api_hook_set_rumble. Conforming gamepads handle **both** encodings;
 *       approximate on ERM hardware as needed.
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
 * @brief Read consolidated gamepad input (buttons, joysticks, triggers) for the current input report.
 * @param out Filled when the hook returns true (#sinput_input_s).
 * @return True if @a out was updated. When false, the library reuses the last cached buttons/sticks/triggers block.
 * @note Weak default returns false; override in your port.
 */
bool sinput_api_hook_get_input(sinput_input_s *out);

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
 * Parses vendor output report ID 3. Forward payloads from USB (**SET_REPORT** / interrupt OUT) or from
 * your **Bluetooth HID** host→device handler-the byte layout is identical.
 *
 * @param data Raw bytes as received from the host (includes report ID in @a data[0] when applicable).
 * @param len  Length of @a data in bytes.
 */
void sinput_api_output_tunnel(const uint8_t *data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif
