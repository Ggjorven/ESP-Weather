#include <Wire.h>
#include <WiFi.h>

#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <PubSubClient.h>

#include <cstring>
#include <string>

#include "secrets.hpp"

namespace Settings
{
    
    // I2C
    inline constexpr static const uint8_t SDAPin = 9u;
    inline constexpr static const uint8_t SCLPin = 8u;

    // LCD
    inline constexpr static const uint8_t LCDColumns = 16u;
    inline constexpr static const uint8_t LCDRows = 2u;

    struct LCDPosition
    {
    public:
        uint8_t Column;
        uint8_t Row;
    };

    inline constexpr static const LCDPosition TemperaturePosition = { 0, 0 };
    inline constexpr static const LCDPosition HumidityPosition = { 6, 0 };
    inline constexpr static const LCDPosition HeatIndexPosition = { 12, 0 };
    inline constexpr static const LCDPosition GasPosition = { 0, 1 };

    // DHT
    inline constexpr static const uint8_t DHTPin = 42u;
    inline constexpr static const auto DHTType = DHT22;

    // MQ2
    inline constexpr static const uint8_t MQ2Pin = 14;
    static uint16_t GasThreshold = 70;

    // LEDs
    inline constexpr static const uint8_t GreenLEDPin = 48;
    inline constexpr static const uint8_t RedLEDPin = 45;

    // MQTT
    inline constexpr static const bool UseMQTT = false;

    inline constexpr static const char* MQTTTemperatureTopic = "JorbenvanderWal/Temperatuur";
    inline constexpr static const char* MQTTHumidityTopic = "JorbenvanderWal/Vochtigheid";
    inline constexpr static const char* MQTTHeatIndexTopic = "JorbenvanderWal/GevoelsTemperatuur";
    inline constexpr static const char* MQTTGasTopic = "JorbenvanderWal/Gas";
    inline constexpr static const char* MQTTGasThresholdTopic = "JorbenvanderWal/GasThreshold";
    inline constexpr static const char* MQTTRefreshRateTopic = "JorbenvanderWal/RefreshRate";

    // Other
    static uint16_t DelayMS = 2000u;

}

namespace Devices
{

    WiFiClient WiFiClient;
    PubSubClient MQTTClient(WiFiClient);

    LiquidCrystal_I2C LCD = LiquidCrystal_I2C(0x27, Settings::LCDColumns, Settings::LCDRows);  
    DHT THHSensor = DHT(Settings::DHTPin, Settings::DHTType);
    auto MQ2Read = []() -> uint16_t { return static_cast<uint16_t>(analogRead(Settings::MQ2Pin)); };

    auto GreenLEDToggle = [](bool enabled = true) -> void { digitalWrite(Settings::GreenLEDPin, enabled); };
    auto RedLEDToggle = [](bool enabled = true) -> void { digitalWrite(Settings::RedLEDPin, enabled); };

}

static void topicCallback(char* topic, byte* payload, unsigned int length) 
{
    static const auto getValue = [](const std::string& strValue, uint16_t& outValue) -> bool
    {
        long unsigned int value32 = std::stoul(strValue);

        if (value32 > UINT16_MAX)
            return false;

        outValue = static_cast<uint16_t>(value32);
        return true;
    };

    if (std::strcmp(Settings::MQTTGasThresholdTopic, topic) == 0)
    {
        std::string strValue(reinterpret_cast<char*>(payload), length);
        uint16_t value;

        if (!getValue(strValue, value))
        {
            Serial.printf("[ERROR] Failed to set new GasThreshold %s...\n", strValue.c_str());
            return;
        }

        Settings::GasThreshold = value;
    }
    else if (std::strcmp(Settings::MQTTRefreshRateTopic, topic) == 0)
    {
        std::string strValue(reinterpret_cast<char*>(payload), length);
        uint16_t value;

        if (!getValue(strValue, value))
        {
            Serial.printf("[ERROR] Failed to set new RefreshRate %sms...\n", strValue.c_str());
            return;
        }

        Settings::DelayMS = value;
    }
}

void setup() 
{
    // Initialize logging
    {
        Serial.begin(115200);
        Serial.println("[DEBUG] Initialized logging.");
    }

    // Initialize I2C
    {
        Wire.begin(Settings::SDAPin, Settings::SCLPin); 
        Serial.printf("[DEBUG] Initialized I2C on pins: %u (SDA), %u (SCL).\n", Settings::SDAPin, Settings::SCLPin);
    }

    // Initialize LCD
    {
        Devices::LCD.init();
        Devices::LCD.backlight();
        Serial.println("[DEBUG] Initialized LCD display (I2C) and turned on backlight.");

        Devices::LCD.setCursor(0, 0);
        Devices::LCD.print("Initializing...");
    }

    // Initialize DHT
    {
        Devices::THHSensor.begin();
        Serial.println("[DEBUG] Started DHT device.");
    }

    // Initialize MQ-2
    {
        analogReadResolution(10); // 10-bit ADC resolution
        pinMode(Settings::MQ2Pin, INPUT);
        Serial.printf("[DEBUG] Initialized MQ-2 on analog pin: %u.\n", Settings::MQ2Pin);
    }

    // Initialize LEDs
    {
        pinMode(Settings::GreenLEDPin, OUTPUT);
        pinMode(Settings::RedLEDPin, OUTPUT);

        Serial.printf("[DEBUG] Initialized Green and RED LEDS on pins: %u, %u.\n", Settings::GreenLEDPin, Settings::RedLEDPin);

        // Set the green LED to on.
        Devices::GreenLEDToggle(true);

        Serial.println("[DEBUG] Toggled green LED.");
    }

    // Initialize WiFi
    if constexpr (Settings::UseMQTT)
    {
        WiFi.begin(String(Secrets::WiFiSSID), String(Secrets::WiFiPassword));
        
        while (WiFi.status() != WL_CONNECTED) 
        {
            delay(500);
            Serial.println("[DEBUG] Connecting to WiFi..");
        }
        
        Serial.printf("[DEBUG] Connected to WiFi network: \"%s\".\n", Secrets::WiFiSSID);
    }

    // Connect to MQTT
    if constexpr (Settings::UseMQTT)
    {
        Devices::MQTTClient.setServer(Secrets::MQTTBroker, Secrets::MQTTPort);
        Devices::MQTTClient.setCallback(&topicCallback);

        while (!Devices::MQTTClient.connected()) 
        {
            std::string clientID = std::string("esp32") + WiFi.macAddress().c_str();
            
            Serial.printf("[DEBUG] Trying to connect to broker as %s.\n", clientID.c_str());

            if (Devices::MQTTClient.connect(clientID.c_str(), Secrets::MQTTUsername, Secrets::MQTTPassword))
            {
                Serial.printf("[DEBUG] Successfully connected to broker %s.\n", Secrets::MQTTBroker);
            } 
            else 
            {
                Serial.printf("[DEBUG] Failed to connect, state: %i, retrying in 2 seconds.\n", Devices::MQTTClient.state());
                delay(2000);
            }
        }
    }

    // Subscribe to MQTT topics
    if constexpr (Settings::UseMQTT)
    {
        static const auto handleResult = [](const bool result, const char* topic) -> void
        {
            if (!result)
                Serial.printf("[DEBUG] Failed to subscribe to topic: \"%s\".\n", topic);
        };

        handleResult(Devices::MQTTClient.subscribe(Settings::MQTTGasThresholdTopic), Settings::MQTTGasThresholdTopic);
        handleResult(Devices::MQTTClient.subscribe(Settings::MQTTRefreshRateTopic), Settings::MQTTRefreshRateTopic);
    }
}

void loop() 
{
    // Poll MQTT events
    if constexpr (Settings::UseMQTT)
    {
        Devices::MQTTClient.loop();
    }

    // Clear LCD
    {
        Devices::LCD.clear();
    }

    // Read & Compute THH values (temperature, humidity, heatIndex)
    float humidity /* XX.XX */, temperature /* XX.XX */, heatIndex /* XX.XX */;
    {
        humidity = Devices::THHSensor.readHumidity();
        temperature = Devices::THHSensor.readTemperature(false); // Note: isFahrenheit = false
        
        // Check if any reads failed and exit early (to try again).
        if (std::isnan(humidity) || std::isnan(temperature)) 
        {
            Serial.println("[ERROR] Failed to read either humidity or temperature.");
            return;
        }
        
        heatIndex = Devices::THHSensor.computeHeatIndex(temperature, humidity, false); // Note: isFahrenheit = false

        Serial.printf("[TRACE] Humidity: %.1f%, ", humidity);
        Serial.printf("Temperature: %.1f°C, ", temperature);
        Serial.printf("HeatIndex: %.1f°C.\n", heatIndex);
    }

    // Read MQ-2
    uint16_t mq2Value;
    {
        mq2Value = Devices::MQ2Read();
        Serial.printf("[TRACE] MQ2 Value: %u.\n", mq2Value);
    }

    // Toggle LEDs
    {
        if (mq2Value >= Settings::GasThreshold)
        {
            Devices::GreenLEDToggle(false);
            Devices::RedLEDToggle(true);
            Serial.println("[TRACE] Green LED set to OFF and Red LED set to ON.");
        }
        else
        {
            Devices::GreenLEDToggle(true);
            Devices::RedLEDToggle(false);
            Serial.println("[TRACE] Green LED set to ON and Red LED set to OFF.");
        }
    }

    // Display values on LCD
    {
        Devices::LCD.setCursor(Settings::TemperaturePosition.Column, Settings::TemperaturePosition.Row);
        Devices::LCD.printf("%.1fC", temperature);

        Devices::LCD.setCursor(Settings::HumidityPosition.Column, Settings::HumidityPosition.Row);
        Devices::LCD.printf("%.1f%%", humidity);

        Devices::LCD.setCursor(Settings::HeatIndexPosition.Column, Settings::HeatIndexPosition.Row);
        Devices::LCD.printf("%.1fC", heatIndex);

        Devices::LCD.setCursor(Settings::GasPosition.Column, Settings::GasPosition.Row);
        Devices::LCD.printf("%u CO2", mq2Value);
    }

    // Publish values to MQTT
    if constexpr (Settings::UseMQTT)
    {
        static const auto handleResult = [](const bool result, const char* topic) -> void
        {
            if (!result)
                Serial.printf("[ERROR] Failed to publish topic: \"%s\".\n", topic);
        };

        handleResult(Devices::MQTTClient.publish(Settings::MQTTTemperatureTopic, std::to_string(temperature).c_str()), Settings::MQTTTemperatureTopic);
        handleResult(Devices::MQTTClient.publish(Settings::MQTTHumidityTopic, std::to_string(humidity).c_str()), Settings::MQTTHumidityTopic);
        handleResult(Devices::MQTTClient.publish(Settings::MQTTHeatIndexTopic, std::to_string(heatIndex).c_str()), Settings::MQTTHeatIndexTopic);
        handleResult(Devices::MQTTClient.publish(Settings::MQTTGasTopic, std::to_string(mq2Value).c_str()), Settings::MQTTGasTopic);
    }

    // Wait 2 seconds 
    delay(Settings::DelayMS);
}