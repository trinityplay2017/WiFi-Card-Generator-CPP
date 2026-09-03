#pragma once

#include <string>

class WifiQrGenerator
{
public:
    // security values:
    //   WPA    = WPA / WPA2 / WPA3-Personal (QR-compatible personal Wi-Fi)
    //   WEP    = WEP
    //   nopass = Open network
    static std::string BuildPayload(const std::string& ssid,
                                    const std::string& password,
                                    const std::string& security);
};
