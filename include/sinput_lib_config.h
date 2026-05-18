/*
 * SINPUT-LIB-HID - Persistent device capability/configuration store used by protocol generation.
 *
 * Copyright (c) 2026 Hand Held Legend, LLC
 * Author: Mitchell Cairns
 *
 * SPDX-License-Identifier: MIT-0
 */

#ifndef SINPUT_LIB_CONFIG_H
#define SINPUT_LIB_CONFIG_H

#include "sinput_lib_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Validates and stores a copy of the device configuration for protocol and feature reports.
 *
 * Out-of-range enum values or too-fast polling may be clamped; see return status.
 *
 * @param cfg Device configuration to copy (must point to a valid #sinput_device_cfg_s). The display name is not part of this struct - it comes from USB string descriptors.
 * @return #SINPUT_CONFIG_OK or #SINPUT_CONFIG_ISSUES_OVERRIDE if values were sanitized.
 */
sinput_config_status_t sinput_config_set(sinput_device_cfg_s *cfg);

/**
 * @brief Copies the active configuration into @a out if initialization completed.
 *
 * @param out Destination; left untouched when NULL or before first successful @ref sinput_config_set.
 */
void sinput_config_get(sinput_device_cfg_s *out);

#ifdef __cplusplus
}
#endif

#endif
