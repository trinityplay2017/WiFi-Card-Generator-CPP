#include "WifiQrGenerator.h"

namespace
{
std::string EscapeWifiField(const std::string& value)
{
    std::string result;
    result.reserve(value.size());

    for (char ch : value)
    {
        switch (ch)
        {
        case '\\':
        case ';':
        case ',':
        case ':':
        case '"':
            result += '\\';
            break;
        default:
            break;
        }

        result += ch;
    }

    return result;
}
}

std::string WifiQrGenerator::BuildPayload(const std::string& ssid,
                                          const std::string& password,
                                          const std::string& security)
{
    const std::string safeSsid = EscapeWifiField(ssid);

    if (security == "nopass")
    {
        return "WIFI:T:nopass;S:" + safeSsid + ";;";
    }

    const std::string safePassword = EscapeWifiField(password);

    return "WIFI:T:" + security +
           ";S:" + safeSsid +
           ";P:" + safePassword + ";;";
}
