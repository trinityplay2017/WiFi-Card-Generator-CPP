# WiFi Card Generator (C++)

Native Windows port of the original **WiFi-Card-Generator**.

## Features

- SSID input
- Selectable WiFi security type
- Live WiFi QR-code generation
- Correct escaping of WiFi QR reserved characters
- Native Win32/GDI user interface
- Printable WiFi card layout
- No JavaScript or browser runtime required
- QR generation provided by the MIT-licensed Nayuki QR-Code-generator C++ library

## Supported security types

The application currently provides these choices:

- **WPA / WPA2 / WPA3-Personal** — encoded as `T:WPA` for broad WiFi QR compatibility
- **WEP** — encoded as `T:WEP`
- **Open / No password** — encoded as `T:nopass`; the password is omitted

The commonly supported WiFi QR syntax defines `WEP`, `WPA`, and `nopass`, while WPA2-EAP has additional enterprise fields. citeturn0search0turn0search1

### WPA3 note

WPA3-Personal does not use a separate universally compatible `T:WPA3` value in the basic WiFi QR format. For this reason the UI groups WPA, WPA2, and WPA3-Personal under the QR-compatible `T:WPA` value. WPA3-specific QR extensions can contain additional fields, but support varies between scanners. citeturn0search11

## WiFi QR format

Examples:

```text
WIFI:T:WPA;S:<SSID>;P:<PASSWORD>;;
WIFI:T:WEP;S:<SSID>;P:<PASSWORD>;;
WIFI:T:nopass;S:<SSID>;;
```

Reserved characters (`\\`, `;`, `,`, `:`, and `"`) are escaped before encoding.

## Build

Requires CMake 3.16+ and a C++11-or-newer compiler.

```text
cmake -S . -B build
cmake --build build --config Release
```

The project uses CMake `FetchContent` to obtain the QR generator source at configure time, so no package manager is required.

## License

The application code in this repository is released under the MIT License.

The QR implementation is from Project Nayuki's **QR-Code-generator** project and is used under its MIT License. See `third_party/NOTICE.txt` for attribution.
