# ESP-Weather

ESP-Weather is a lightweight, extensible weather station project for the **ESP32**, designed to collect environmental data from sensors (such as **DHT22** and others) and publish the readings to an **MQTT** server for storage, visualization, or automation.
  
The project focuses on simplicity making it easy to deploy a standalone IoT weather node or integrate it into a larger home-automation or data-logging setup.

## Features

- **ESP32-Based Weather Station**: Built specifically for the ESP32 platform, leveraging its Wi-Fi capabilities and low power consumption.

- **Sensor Support (DHT22 + More)**: Reads temperature and humidity from a DHT22 sensor, with a structure designed to easily add additional sensors (pressure, air quality, light, etc.).

- **MQTT Data Publishing**: Sends sensor data to an MQTT broker, enabling seamless integration with Home Assistant, Node-RED, InfluxDB, Grafana, or custom backends.

- **Configurable Update Intervals**: Control how often sensor data is sampled and published to balance responsiveness and power usage.

- **Simple, Readable Codebase**: Prioritizes clarity over cleverness—easy to understand, modify, and extend.

- **Low Resource Footprint**: Designed to run comfortably within the memory and performance constraints of the ESP32.

## Getting Started

### Prerequisites

Ensure you have the following installed on your system:

- **Arduino IDE**
- **ESP32 S3 Dev kit**
- **LiquidCrystal by Macro Schwartz** ([tutorial](https://randomnerdtutorials.com/esp32-esp8266-i2c-lcd-arduino-ide/))
- **PubSubClient by knolleary**
- **DHT Sensor by Adafruit**

### Hardware Setup

// TODO: ...

### Building

1. Clone the repository:
   ```
   git clone https://github.com/GGjorven/ESP-Weather.git
   ```

2. Replace the placeholders in [secrets.hpp](main/secrets.hpp)

3. Open the main.ino with the **Arduino IDE**

4. Upload!

## TODO List
- [x] Upload main.ino
- [x] Upload secrets.hpp with placeholders
- [x] Create .gitignore with secrets.hpp
- [ ] Replace std::string with String

## License

This project is licensed under the Apache 2.0 License. See [LICENSE](LICENSE) for details.

## Contributing

Contributions are welcome! Please fork the repository and create a pull request with your changes.

## Third-Party Libraries
- [LiquidCrystal](https://github.com/johnrickman/LiquidCrystal_I2C) - LCD handling library
- [PubSubClient](https://github.com/g-truc/glmhttps://github.com/knolleary/pubsubclient) - MQTT library
- [DHT Sensor](https://github.com/adafruit/DHT-sensor-library) - DHT library
