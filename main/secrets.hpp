#pragma once

#include <cstdint>

namespace Secrets
{

    inline constexpr static const char* WiFiSSID = "<REPLACE>";
    inline constexpr static const char* WiFiPassword = "<REPLACE>";

    inline constexpr static const char* MQTTBroker = "test.mosquitto.org";
    inline constexpr static const uint16_t MQTTPort = 1883;

    // Note: These are allowed to be empty if no username and password are required.
    inline constexpr static const char* MQTTUsername = "";
    inline constexpr static const char* MQTTPassword = "";

}