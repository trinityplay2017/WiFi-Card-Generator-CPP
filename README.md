# WiFi Card Generator (C++)

Native Windows port of the original **WiFi-Card-Generator**.

## Features

- SSID and password input
- Live WiFi QR-code generation
- Correct escaping of WiFi QR reserved characters
- Native Win32/GDI user interface
- Printable WiFi card layout
- No JavaScript or browser runtime required
- QR generation provided by the MIT-licensed Nayuki QR-Code-generator C++ library

## Build

Requires CMake 3.16+ and a C++11-or-newer compiler.

```text
cmake -S . -B build
cmake --build build --config Release
```

The project uses CMake `FetchContent` to obtain the QR generator source at configure time, so no package manager is required.

## WiFi QR format

The generator produces the standard WiFi payload:

`WIFI:T:WPA;S:<SSID>;P:<PASSWORD>;;`

Reserved characters (`\\`, `;`, `,`, `:`, and `"`) are escaped before encoding.

## License

The application code in this repository is released under the MIT License.

The QR implementation is from Project Nayuki's **QR-Code-generator** project and is used under its MIT License. See `third_party/NOTICE.txt` for attribution.
