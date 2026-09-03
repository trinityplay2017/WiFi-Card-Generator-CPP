#pragma once

#include <string>

class WifiQrGenerator
{
public:
    static std::string BuildPayload(const std::string& ssid, const std::string& password);
};
