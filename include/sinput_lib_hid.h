/*
 * SINPUT-LIB-HID — USB HID report and configuration descriptors for enumeration.
 *
 * Copyright (c) 2026 Hand Held Legend, LLC
 * Author: Mitchell Cairns
 *
 * SPDX-License-Identifier: MIT-0
 */

#ifndef SINPUT_LIB_HID_H
#define SINPUT_LIB_HID_H

#include <stdint.h>

#include "sinput_lib_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Returns the built-in USB device descriptor (VID/PID, strings index, etc.).
 * @return Pointer to a statically allocated #sinput_usb_device_descriptor_t.
 */
const sinput_usb_device_descriptor_t *sinput_hid_get_device_descriptor(void);

/**
 * @brief Provides pointers and lengths for HID report, configuration descriptors, and default VID/PID.
 *
 * Any output pointer may be NULL if that value is not needed. Lengths are byte counts for the binary blobs.
 *
 * @param[out] hid_report_descriptor       Start of the HID report descriptor bytes.
 * @param[out] hid_report_descriptor_len    Length of the HID report descriptor.
 * @param[out] configuration_descriptor     Full-speed configuration descriptor including interface/HID/endpoints.
 * @param[out] configuration_descriptor_len Length of the configuration descriptor.
 * @param[out] vid                          Vendor ID (default Raspberry Pi allocation unless customized in C).
 * @param[out] pid                          Product ID for this firmware’s gamepad profile.
 */
void sinput_hid_get_descriptor_params(const uint8_t **hid_report_descriptor, uint16_t *hid_report_descriptor_len,
                                  const uint8_t **configuration_descriptor, uint16_t *configuration_descriptor_len,
                                  uint16_t *vid, uint16_t *pid);

#ifdef __cplusplus
}
#endif

#endif
