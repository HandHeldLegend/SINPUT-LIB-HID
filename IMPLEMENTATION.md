# SINPUT-LIB-HID - Implementation guide

This document is for firmware authors integrating **SINPUT-LIB-HID** over **USB** and/or **Bluetooth HID** (Classic or LE HID-over-GATT) on **any platform** - any path that carries the same **64-byte HID framing** for input and output. The library is **transport- and MCU-agnostic**; your stack (e.g. TinyUSB, vendor USBD, BTstack, NimBLE) owns enumeration and pipes. This guide describes the **contract** between the library and your code; it does not replace reading the headers for exact type definitions.

---

## 1. Architecture

```
┌─────────────────────────────────────────────────────────────┐
│  Your transport (USB and/or Bluetooth HID, etc.)              │
│  • IN:  emit 64-byte HID reports                            │
│  • OUT: deliver host payloads to sinput_api_output_tunnel   │
└───────────────┬─────────────────────────────┬───────────────┘
                │                             │
                ▼                             ▼
     sinput_api_generate_inputreport   sinput_api_output_tunnel
                │                             │
                └──────────────┬──────────────┘
                               ▼
              Protocol + config (inside the library)
                               ▼
              Weak hooks: sinput_api_hook_get_* / set_*
                               ▼
┌─────────────────────────────────────────────────────────────┐
│  Your board support (GPIO, ADC, IMU, motors, LEDs, …)       │
└─────────────────────────────────────────────────────────────┘
```

The library **does not** open USB endpoints, Bluetooth links, or start timers. It only:

- Builds the **wire-format** buffer when you ask.
- Parses **vendor output** commands when you forward bytes.
- Stores **device configuration** after `sinput_api_init()` / `sinput_config_set()`.

---

## 2. CMake and includes

- Link **`sinput_lib_hid`** (see [`README.md`](README.md)). The CMake target carries **no** vendor SDK link dependency - only `include/` usage and C11.
- Include **`sinput_lib.h`** for normal application use.
- Include **`sinput_lib_hid.h`** where you register **USB** descriptors.

You do not need to include the protocol or config headers unless you call `sinput_protocol_*` or `sinput_config_*` directly (the public path is the `sinput_api_*` layer).

---

## 3. Initialization and configuration

### 3.1 Populate `sinput_device_cfg_s`

Set fields to describe **what the hardware actually has** (buttons, sticks, IMU, touchpads, rumble, LEDs, etc.) and identity fields such as:

- `polling_rate_us` - Intended report period in microseconds.
- `mac_address` - Six bytes used as MAC or serial in the protocol.
- Enums for gamepad style / face layout / handheld vs joypad format.

**Display name on PC:** This struct does **not** carry a product name. APIs such as **SDL3** and **Steam Input** take the user-visible controller name from **USB string descriptors** on the wired path (typically the string selected by the device descriptor’s **product** index). Your USB stack must still return those Unicode strings for GET_DESCRIPTOR(STRING); the indices in `sinput_hid_get_device_descriptor()` must match the strings you register. Over **Bluetooth HID**, expose a consistent device/service name through your stack (e.g. SDP/HID record or GATT device name as your platform requires)-that is the analogue hosts use when there is no USB descriptor fetch.

**Important:** `sinput_api_init()` forwards to `sinput_config_set()`, which copies from your pointer with `memcpy`. Pass a **valid, non-NULL** pointer to a fully initialized struct. The library does not accept NULL.

### 3.2 Call `sinput_api_init()`

```c
sinput_device_cfg_s cfg = { /* … */ };
sinput_config_status_t st = sinput_api_init(&cfg);
```

Return values (`sinput_config_status_t`):

- **`SINPUT_CONFIG_OK`** - Stored as-is.
- **`SINPUT_CONFIG_ISSUES_OVERRIDE`** - One or more fields were **sanitized** (for example polling rate too low, enum out of range). Firmware keeps running; treat this as a warning in debug builds.

After a successful set, internal state is marked ready; `sinput_config_get()` can read back the active configuration (used when generating feature reports).

---

## 4. Descriptors and transport roles

### 4.1 USB

Before the device may enumerate as this gamepad profile over **USB**:

1. Call **`sinput_hid_get_device_descriptor()`** for the USB **device** descriptor structure (VID/PID, string indices, etc.).
2. Call **`sinput_hid_get_descriptor_params()`** to obtain pointers and byte lengths for:

   - HID **report** descriptor  
   - Full-speed **configuration** descriptor (interface + HID + endpoints)

Use these blobs with your USB device stack the same way you would for any HID class. Default VID/PID values are **compiled into** `sinput_lib_hid.c`; change them in source if your product uses different USB-IF allocations.

**Strings** (manufacturer, product, serial) are referenced by index in the device descriptor (`iManufacturer`, `iProduct`, `iSerialNumber`). Your USB stack must implement **GET_DESCRIPTOR(STRING)** so those indices resolve to the correct UTF-16LE strings. That is how hosts (including **SDL3** and **Steam Input**) obtain the **device name** shown to users on the wired path - it is not duplicated inside `sinput_device_cfg_s`.

### 4.2 Bluetooth HID

SINPUT’s **report bytes and output-tunnel layout are the same** whether traffic runs over **USB** or **Bluetooth HID**. Helpers in `sinput_lib_hid.h` are **USB-oriented** (device + configuration descriptors). For **Bluetooth**:

- Register the **same** HID **report descriptor** content with your Bluetooth HID implementation (Classic HID SDP record and/or **HID over GATT** service, depending on your stack).
- **Input:** when your firmware sends a HID input report to the host (interrupt channel, notification, or equivalent), fill the buffer with **`sinput_api_generate_inputreport()`** first-identical 64-byte framing to USB.
- **Output:** when the host sends a HID output report on the Bluetooth HID path, pass those bytes to **`sinput_api_output_tunnel()`** exactly as for USB SET_REPORT / OUT.

Your Bluetooth stack-not this library-owns pairing, connection interval, and channel mapping.

---

## 5. Device → host: input reports

### 5.1 When to call

Call **`sinput_api_generate_inputreport(uint8_t out[64])`** at the rate implied by your `polling_rate_us` and your transport (**USB** interrupt interval or **Bluetooth HID** send cadence, e.g. connection event / poll). The buffer must be **64 bytes**.

### 5.2 What happens inside

- If the library is servicing a **feature-discovery** request (see §7), the next call may produce a **report ID 0x02** style payload instead of normal gamepad samples.
- Otherwise the function collects data via **getter hooks** and fills the main gamepad report (**report ID 0x01** in the HID descriptor): power, consolidated gamepad input (`sinput_input_s`), motion, and touchpads—each only when the corresponding hook returns success.

### 5.3 Getter hooks (implement on your board)

Implement any of these by **defining** the same symbol as the weak default in `sinput_lib.c` (same name and signature):

| Hook | Purpose |
|------|---------|
| `sinput_api_hook_get_power` | Battery % and connection / charging status. |
| `sinput_api_hook_get_input` | Digital buttons, stick axes, and **raw** trigger values in one `sinput_input_s` snapshot; library maps triggers from 0…4095 into the wire range. |
| `sinput_api_hook_get_motion` | Accel, gyro, timestamp. |
| `sinput_api_hook_get_touchpads` | Left/right pad coordinates and pressure. |

Return **`true`** when you filled the struct; **`false`** if that hook has nothing new or the data is unsupported. The library leaves prior internal state in place when a hook returns false (see source for static caching on the main input path). For `sinput_api_hook_get_input`, buttons, sticks, and triggers are updated **together**—there is no per-axis caching split across separate hooks.

---

## 6. Host → device: output tunnel

### 6.1 Entry point

Forward **complete** HID output frames to:

```c
void sinput_api_output_tunnel(const uint8_t *data, uint16_t len);
```

### 6.2 Filtering rules (current implementation)

The handler **ignores** packets unless:

- `len >= 2`, and  
- **`data[0] == 0x03`** - vendor **output report ID** (`REPORT_ID_SINPUT_OUTPUT_CMDDAT`).

Pass through the bytes your stack delivers **unchanged**-often the first byte is the report ID (`data[0]`) on both USB and **Bluetooth HID** output paths.

### 6.3 Command byte

`data[1]` selects the command:

| Value | Constant (conceptual) | Action |
|------:|------------------------|--------|
| `0x01` | Haptic | Decodes dual-motor payloads and calls **`sinput_api_hook_set_haptics`** or **`sinput_api_hook_set_rumble`** depending on type. |
| `0x02` | Features | Arms a **one-shot** feature report on the **next** input generation (see §7). |
| `0x03` | Player LED | Calls **`sinput_api_hook_set_player_leds`** with `data[2]`. |
| `0x04` | Joystick RGB | Reads a little-endian **32-bit RGB** from `data[2…5]`, calls **`sinput_api_hook_set_joystick_rgb`**. |

**Haptics and ERM rumble:** Command `0x01` carries a **type** field that routes to **`sinput_api_hook_set_haptics`** or **`sinput_api_hook_set_rumble`**. Hosts are free to use either encoding depending on title or driver. **SINPUT gamepads are expected to handle both paths properly in firmware** - if you only have ERM motors, still implement the haptics hook (for example by mapping frequencies and amplitudes into motor drive), and vice versa for HD actuators receiving ERM-style packets. Weak defaults are no-ops; production firmware should override **both** hooks when ship criteria require correct behavior across games.

Setter hooks are **weak**; define them like the getters.

### 6.4 USB device stack (example: TinyUSB)

- For **Interrupt OUT** or **SET_REPORT** on the gamepad interface, copy the payload into a buffer and call `sinput_api_output_tunnel()`.
- Keep the path **fast**: do not block inside the callback; defer heavy work to your main loop if needed.

### 6.5 Bluetooth HID

- Forward **host→device** HID output data from your Bluetooth stack’s HID control/output handler to **`sinput_api_output_tunnel()`** using the same byte layout as USB.
- Keep callbacks **light**; defer heavy work to your main loop if needed.

---

## 7. Feature discovery handshake

Some hosts send command **`0x02`** on the vendor output report to request capability metadata.

1. Output tunnel sets an internal latch when it sees command `0x02` and the latch is idle.
2. The **next** `sinput_api_generate_inputreport()` emits a **feature-style** buffer (report ID **0x02** path in the protocol) built from **`sinput_config_get()`** and capability bitmask helpers, then clears the latch.

No separate “get feature” API is required from your stack if your transport already mirrors HID SET_REPORT/GET_REPORT behavior (including the **Bluetooth HID** equivalent); the library folds the reply into the **next input report generation** as implemented today.

---

## 8. Coordinate and scaling conventions

- **Sticks:** `int16_t` per axis in `sinput_input_s.joysticks`; the HID report descriptor uses 16-bit logical ranges consistent with full-scale joystick values.
- **Triggers:** Provide **unsigned** raw values in `sinput_input_s.triggers` (`.left` / `.right`; implementation maps `[0, 4095]` into the packed report range; values above 4095 are clamped).
- **Motion:** Units expected by the host profile match the structs in `sinput_lib_types.h`; align your IMU fusion with those fields.
- **Buttons:** Use the bitfield layout in `sinput_input_s.buttons` - match south/east/west/north and cluster assignments to your physical PCB.

---

## 9. Threading and reentrancy

Assume **no concurrent** calls into the same APIs from ISR and main unless you add your own locking. Typical pattern: minimal work in the USB or **Bluetooth** callback (queue a flag), build the report and send from a **single** producer context.

---

## 10. Debugging checklist

| Symptom | Things to verify |
|--------|---------------------|
| Host sees no gamepad | **USB:** descriptors/endpoints; `sinput_hid_get_descriptor_params` correct. **Bluetooth:** HID service up; same report descriptor registered; IN notifications or interrupt channel sending 64-byte reports. |
| Buttons always zero | `sinput_api_hook_get_input` defined and returns `true`; `.buttons` bit packing matches `sinput_input_s`. |
| Sticks / triggers stuck | `sinput_api_hook_get_input` returns `true`; fill `.joysticks` / `.triggers`; trigger raw values in 0…4095. |
| No rumble / LEDs | Output path calls `sinput_api_output_tunnel`; first byte `0x03`; setter hooks implemented (**USB OUT** or **Bluetooth HID** host-to-device path wired). |
| Odd feel only in some games | Host may send **haptics** or **ERM** payloads; implement **both** `sinput_api_hook_set_haptics` and `sinput_api_hook_set_rumble` (see §6.3). |
| Feature/Capabilities wrong | `sinput_api_init` succeeded; `sinput_device_cfg_s` reflects hardware; wait one input cycle after feature command. |

---

## 11. API quick reference

| Function | Role |
|----------|------|
| `sinput_api_init` | Store device configuration (calls `sinput_config_set`). |
| `sinput_api_generate_inputreport` | Build 64-byte HID buffer for device→host (USB IN or Bluetooth HID send). |
| `sinput_api_output_tunnel` | Parse host→device payload (USB OUT / SET_REPORT or Bluetooth HID output). |
| `sinput_hid_get_device_descriptor` | USB device descriptor pointer. |
| `sinput_hid_get_descriptor_params` | HID + configuration descriptor blobs and lengths. |

---

## 12. Further reading

- [`include/sinput_lib.h`](include/sinput_lib.h) - Annotated public API.  
- [`include/sinput_lib_types.h`](include/sinput_lib_types.h) - All public structs and enums.  
