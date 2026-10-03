# ESP_MQTT_JAI_VERMA
This project is an ESP32-based IoT system that allows a motor to be controlled remotely through a web dashboard while also monitoring temperature and humidity.

The ESP32 connects to the internet and communicates with an EMQX Cloud MQTT broker. From the web dashboard, a user can turn the motor ON or OFF, and the command is sent to the ESP32 through MQTT. A relay connected to the ESP32 then controls the motor. At the same time, a DHT22 sensor continuously measures temperature and humidity and sends the readings to the dashboard.

The project was built to understand and implement real-world IoT communication using ESP32, MQTT, EMQX Cloud, sensors, relay control, and a web-based interface.
## Main Components & Technologies

- **ESP32** – Main microcontroller and IoT device
- **DHT22** – Temperature and humidity sensing
- **Relay Module** – Motor ON/OFF control
- **MQTT** – Communication protocol between the ESP32, broker, and dashboard
- **EMQX Cloud** – Cloud-hosted MQTT broker for message communication
- **Netlify** – Hosting platform for the web dashboard
