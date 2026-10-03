#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "DHT.h"

// Wi-Fi
const char* WIFI_SSID = "AirFiber-PR0voH";
const char* WIFI_PASSWORD = "PASSWORD";

// EMQX Cloud
const char* MQTT_SERVER = "ac21711a.ala.asia-southeast1.emqxsl.com";
const int MQTT_PORT = 8883;
const char* MQTT_USER = "esp32_new";
const char* MQTT_PASSWORD = "PASSWORD";

// MQTT topics
const char* MOTOR_TOPIC = "iot/jai/esp32-001/motor";
const char* MOTOR_STATUS_TOPIC = "iot/jai/esp32-001/motor/status";
const char* TEMPERATURE_TOPIC = "iot/jai/esp32-001/temperature";

// Relay
const int RELAY_PIN = 4;
const int RELAY_OFF = LOW;
const int RELAY_ON = HIGH;
bool motorState = false;

// DHT22
#define DHT_PIN 13
#define DHT_TYPE DHT22
DHT dht(DHT_PIN, DHT_TYPE);

// MQTT
WiFiClientSecure espClient;
PubSubClient mqtt(espClient);

// Timing
unsigned long lastDHTRead = 0;
const unsigned long DHT_INTERVAL = 5000;

void mqttCallback(char* topic, byte* payload, unsigned int length)
{
    String command = "";

    for (unsigned int i = 0; i < length; i++)
    {
        command += (char)payload[i];
    }

    command.trim();

    Serial.println();
    Serial.println("MQTT command received:");
    Serial.print("Topic: ");
    Serial.println(topic);
    Serial.print("Command: ");
    Serial.println(command);

    if (command == "ON")
    {
        digitalWrite(RELAY_PIN, RELAY_ON);
        motorState = true;

        Serial.println("Motor: ON");

        mqtt.publish(
            MOTOR_STATUS_TOPIC,
            "ON",
            true
        );
    }
    else if (command == "OFF")
    {
        digitalWrite(RELAY_PIN, RELAY_OFF);
        motorState = false;

        Serial.println("Motor: OFF");

        mqtt.publish(
            MOTOR_STATUS_TOPIC,
            "OFF",
            true
        );
    }
    else
    {
        Serial.println("Unknown command");
    }
}

void connectWiFi()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        return;
    }

    Serial.print("Connecting to Wi-Fi");

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("Wi-Fi connected");

    Serial.print("ESP32 IP Address: ");
    Serial.println(WiFi.localIP());
}

void connectMQTT()
{
    while (!mqtt.connected())
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            connectWiFi();
        }

        Serial.print("Connecting to EMQX... ");

        if (mqtt.connect(
                "ESP32-001",
                MQTT_USER,
                MQTT_PASSWORD))
        {
            Serial.println("CONNECTED");

            if (mqtt.subscribe(MOTOR_TOPIC))
            {
                Serial.print("Subscribed to: ");
                Serial.println(MOTOR_TOPIC);
            }
            else
            {
                Serial.println("Motor topic subscription FAILED");
            }

            mqtt.publish(
                MOTOR_STATUS_TOPIC,
                motorState ? "ON" : "OFF",
                true
            );

            Serial.print("Motor status topic: ");
            Serial.println(MOTOR_STATUS_TOPIC);
        }
        else
        {
            Serial.print("FAILED, error=");
            Serial.println(mqtt.state());

            delay(3000);
        }
    }
}

void publishDHTData()
{
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity))
    {
        Serial.println("DHT22 read failed");
        return;
    }

    String data =
        "{\"temperature\":" +
        String(temperature, 2) +
        ",\"humidity\":" +
        String(humidity, 2) +
        "}";

    if (mqtt.connected())
    {
        bool published = mqtt.publish(
            TEMPERATURE_TOPIC,
            data.c_str()
        );

        Serial.print("DHT22: ");
        Serial.println(data);

        Serial.print("Temperature publish: ");
        Serial.println(published ? "OK" : "FAILED");
    }
}

void setup()
{
    Serial.begin(115200);

    delay(1000);

    Serial.println();
    Serial.println("====================================");
    Serial.println("ESP32 MOTOR + DHT22 IoT CONTROL");
    Serial.println("====================================");

    pinMode(RELAY_PIN, OUTPUT);

    digitalWrite(RELAY_PIN, RELAY_OFF);
    motorState = false;

    Serial.println("Relay initialized");
    Serial.println("Motor: OFF");

    dht.begin();

    Serial.print("DHT22 DATA GPIO: ");
    Serial.println(DHT_PIN);

    connectWiFi();

    // Prototype/testing only.
    // Production should use the EMQX CA certificate.
    espClient.setInsecure();

    mqtt.setServer(
        MQTT_SERVER,
        MQTT_PORT
    );

    mqtt.setCallback(mqttCallback);

    connectMQTT();

    Serial.println();
    Serial.println("System ready");
    Serial.println("Motor command topic:");
    Serial.println(MOTOR_TOPIC);
    Serial.println("Motor status topic:");
    Serial.println(MOTOR_STATUS_TOPIC);
    Serial.println("DHT22 topic:");
    Serial.println(TEMPERATURE_TOPIC);
}

void loop()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Wi-Fi disconnected. Reconnecting...");
        connectWiFi();
    }

    if (!mqtt.connected())
    {
        Serial.println("MQTT disconnected. Reconnecting...");
        connectMQTT();
    }

    mqtt.loop();

    unsigned long currentMillis = millis();

    if (currentMillis - lastDHTRead >= DHT_INTERVAL)
    {
        lastDHTRead = currentMillis;
        publishDHTData();
    }

    delay(10);
}
