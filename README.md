# SINPUT-LIB-HID

**SINPUT-LIB-HID** is a **platform-agnostic** C library for embedded gamepad firmware (any MCU, RTOS, or bare metal). It owns the **HID report layout**, **USB HID / configuration descriptor blobs**, and **host ↔ device command framing** for a 64-byte vendor gamepad profile. It does **not** depend on Raspberry Pi Pico, RP2040, or the Pico SDK. The same **`sinput_api_generate_inputreport()` / `sinput_api_output_tunnel()`** contract applies over **wired USB** and over **Bluetooth HID** (interrupt IN/OUT or the equivalent your stack exposes) - wire the tunnels from whichever transport you ship. You supply hardware-specific behavior by **overriding weak hook functions** (inputs, rumble, LEDs, etc.).

Copyright (c) 2026 Hand Held Legend, LLC.  
Author: Mitchell Cairns.  
Licensed under **MIT-0** - see [`LICENSE`](LICENSE).

---

## What you get

- **Structured gamepad inputs** - Buttons, dual sticks, analog triggers (12-bit style values scaled internally), IMU, touchpads, and power status mapped into a fixed HID input report.
- **Host output handling** - Haptics (HD-style dual pair or ERM-style rumble), player LEDs, joystick RGB, and a **feature-discovery handshake** the host can trigger via output reports.
- **Dual-channel haptics contract** - Hosts may send **either** HD-style haptics (`sinput_api_hook_set_haptics`) **or** ERM-style rumble (`sinput_api_hook_set_rumble`). Conforming SINPUT firmware is expected to **handle both encodings correctly** - even if the hardware only has one class of actuator (for example map or approximate to your motors or voice coils).
- **Descriptors** - Prebuilt **HID report descriptor**, **configuration descriptor**, and **USB device descriptor** accessors for **USB** enumeration (VID/PID defaults are in firmware sources). Over **Bluetooth HID**, you reuse the same **HID report descriptor** with your profile/SDP (or GATT) registration - the library does not open radios itself.
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
| [`CMakeLists.txt`](CMakeLists.txt) | Static library target `sinput_lib_hid` (portable C11; no vendor SDK link dependency). |

---

## Building (CMake)

The CMake target **does not** link a chip SDK or `pico_stdlib`. You link this library next to your own C runtime / startup (or your parent library that already pulls them in).

### Standalone

```cmake
add_subdirectory(path/to/SINPUT-LIB-HID)
target_link_libraries(your_firmware PRIVATE sinput_lib_hid)
# Your project must supply the C library and any HAL your port uses.
```

### Example: Pico SDK / RP2040

If you use the Raspberry Pi Pico SDK, link `sinput_lib_hid` from your firmware target **after** `pico_sdk_init()` like any other static library (same pattern as other portable deps).

### Via HOJA-LIB-RP2040

When your project consumes **HOJA-LIB-RP2040**, SINPUT is already added as an INTERFACE dependency on `hoja_lib`; linking `hoja_lib` pulls in `sinput_lib_hid` automatically (alongside HOJA's own Pico / hardware libraries).

---

## Integration at a glance

1. **Initialize configuration** - Fill a `sinput_device_cfg_s` (caps, polling period, MAC/serial, etc.) and call `sinput_api_init()`. The product name users see comes from **USB string descriptors** (wired) or your **Bluetooth**-visible name/string records (SDL3 / Steam Input use those), not from this struct.
2. **USB** - Register the device, HID, and configuration descriptors from `sinput_hid_get_device_descriptor()` and `sinput_hid_get_descriptor_params()`.
3. **Bluetooth HID** - Register the **same** HID report descriptor with your stack’s HID-over-GATT or Classic HID service. You still call **`sinput_api_generate_inputreport()`** and **`sinput_api_output_tunnel()`** from your Bluetooth HID callbacks - the framing matches USB (see **[`IMPLEMENTATION.md`](IMPLEMENTATION.md)**).
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

- **Language**: ISO C11 (see `CMakeLists.txt` `C_STANDARD`).
- **Portability**: **No dependency** on RP2040, Pico SDK, TinyUSB, or Bluetooth stacks in the library itself - only standard C and the usual libc for your toolchain (`memcpy`, `memset`, etc.). Your firmware wires USB/Bluetooth and hardware.
- **Toolchain note**: Sources use **`__attribute__((weak))`** for hook stubs (GCC/Clang style). If you use another compiler, provide equivalent weak symbols or replace the stubs in your build.

---

## Contributing / forks

Keep copyright and SPDX notices in new files; match the project’s license if you redistribute the library.
