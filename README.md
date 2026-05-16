# SINPUT-LIB-HID

**SINPUT-LIB-HID** is a small C library for embedded gamepad firmware: it owns the **HID report layout**, **USB HID / configuration descriptor blobs**, and **host ↔ device command framing** for a 64-byte vendor gamepad profile. The same **`sinput_api_generate_inputreport()` / `sinput_api_output_tunnel()`** contract applies over **wired USB** and over **Bluetooth HID** (interrupt IN/OUT or the equivalent your stack exposes)-wire the tunnels from whichever transport you ship. You supply hardware-specific behavior by **overriding weak hook functions** (inputs, rumble, LEDs, etc.).

Copyright (c) 2026 Hand Held Legend, LLC.  
Author: Mitchell Cairns.  
Licensed under **MIT-0** - see [`LICENSE`](LICENSE).

---

## What you get

- **Structured gamepad inputs** - Buttons, dual sticks, analog triggers (12-bit style values scaled internally), IMU, touchpads, and power status mapped into a fixed HID input report.
- **Host output handling** - Haptics (HD-style dual pair or ERM-style rumble), player LEDs, joystick RGB, and a **feature-discovery handshake** the host can trigger via output reports.
- **Descriptors** - Prebuilt **HID report descriptor**, **configuration descriptor**, and **USB device descriptor** accessors for **USB** enumeration (VID/PID defaults are in firmware sources). Over **Bluetooth HID**, you reuse the same **HID report descriptor** with your profile/SDP (or GATT) registration-the library does not open radios itself.
- **Porting model** - Weak symbols for all platform hooks; implement only what your hardware exposes.

---

## Repository layout

| Path | Role |
|------|------|
| [`include/sinput_lib.h`](include/sinput_lib.h) | Public API: hooks, `sinput_api_init`, report generation, output tunnel. |
| [`include/sinput_lib_types.h`](include/sinput_lib_types.h) | Enums and structs: device caps, buttons, motion, config blob. |
| [`include/sinput_lib_hid.h`](include/sinput_lib_hid.h) | USB descriptor pointers and lengths. |
| [`include/sinput_lib_protocol.h`](include/sinput_lib_protocol.h) | Lower-level protocol entry points (usually called via the `sinput_api_*` wrappers). |
| [`include/sinput_lib_config.h`](include/sinput_lib_config.h) | Runtime configuration get/set used by the protocol layer. |
| [`CMakeLists.txt`](CMakeLists.txt) | Static library target `sinput_lib_hid` (Pico SDK / `pico_stdlib`). |

---

## Building (CMake)

### Standalone in a Pico SDK project

Add the library directory and link the target your executable (or intermediate library) already links against Pico SDK:

```cmake
add_subdirectory(path/to/SINPUT-LIB-HID)
target_link_libraries(your_firmware PRIVATE sinput_lib_hid)
```

`sinput_lib_hid` exports the **include directory** as PUBLIC; link **after** `pico_sdk_init()` / your board setup as usual.

### Via HOJA-LIB-RP2040

When your project consumes **HOJA-LIB-RP2040**, SINPUT is already added as an INTERFACE dependency on `hoja_lib`; linking `hoja_lib` pulls in `sinput_lib_hid` automatically.

---

## Integration at a glance

1. **Initialize configuration** - Fill a `sinput_device_cfg_s` (caps, polling period, MAC/serial, etc.) and call `sinput_api_init()`. The product name users see comes from **USB string descriptors** (wired) or your **Bluetooth**-visible name/string records (SDL3 / Steam Input use those), not from this struct.
2. **USB** - Register the device, HID, and configuration descriptors from `sinput_hid_get_device_descriptor()` and `sinput_hid_get_descriptor_params()`.
3. **Bluetooth HID** - Register the **same** HID report descriptor with your stack’s HID-over-GATT or Classic HID service. You still call **`sinput_api_generate_inputreport()`** and **`sinput_api_output_tunnel()`** from your Bluetooth HID callbacks-the framing matches USB (see **[`IMPLEMENTATION.md`](IMPLEMENTATION.md)**).
4. **Device → host** - Whenever you send a gamepad IN report (USB interrupt IN or **Bluetooth HID** interrupt/send), build it with `sinput_api_generate_inputreport(buf)` and transmit the **64-byte** buffer.
5. **Host → device** - Whenever the host delivers an HID output report (**SET_REPORT / OUT** on USB, or the **Bluetooth HID** output/control path your stack uses), forward the bytes unchanged to `sinput_api_output_tunnel(data, len)`.
6. **Hooks** - Implement the weak `sinput_api_hook_*` symbols for your board (GPIO, ADC, IMU, motor drivers, LEDs).

For step-by-step wiring, report IDs, and packet layouts, see **[`IMPLEMENTATION.md`](IMPLEMENTATION.md)**.

---

## Documentation

| Document | Contents |
|----------|----------|
| **[`IMPLEMENTATION.md`](IMPLEMENTATION.md)** | Full porting guide: hooks, reports, output commands, feature flow, checklist. |
| Headers | Doxygen-style comments on public symbols (`@brief`, `@param`). |

---

## Requirements

- **Toolchain**: C11 (or what your Pico SDK / board preset uses).
- **Platform**: Written for **Raspberry Pi Pico / RP2040**–style builds via **`pico_stdlib`**. Porting to another RTOS or bare metal requires replacing that link line in `CMakeLists.txt` with your own libc / startup dependencies.

---

## Contributing / forks

Keep copyright and SPDX notices in new files; match the project’s license if you redistribute the library.
