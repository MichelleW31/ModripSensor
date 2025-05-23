

// LIBRARIES
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiClientSecure.h>

#include <IRremote.hpp>

#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include "Adafruit_BME680.h"

#include "secrets.h"

// MQTT SERVER
const char* mqtt_server ="902e1d0dba3944fa88c5f6caac765b57.s1.eu.hivemq.cloud";

// SERVER CONNECTION
const char* modId = "67e1f54045d20e4703ad5c4a";

const char* ca_cert = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";

String idToken;

/////////////////////////////////
///////// BME SENSOR ////////////
/////////////////////////////////

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME680 bme(&Wire);

WiFiClientSecure espSecureClient;
PubSubClient client(espSecureClient);

// Set timer to 10 minutes
// int timerDelay = 600000;
// Set timer to 5 minutes
// int timerDelay = 300000;
// // Set timer to 1 minutes
// int timerDelay = 60000;
// // Set timer to 1 second
int timerDelay = 1000;

//////////////////////////////////
//////////// SETUP ///////////////
//////////////////////////////////

void setup() {
  // Setup Serial Monitor
  Serial.begin(115200);

  // Connect SDA and SCL pins
  Wire.begin(21, 22);

  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms

  // Start wifi 
  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to WiFi network");

  // Connect to mqtt server
  espSecureClient.setCACert(ca_cert);
  client.setServer(mqtt_server, 8883);


  reconnect();
}

//////////////////////////////////
/////// RECONNECT TO MQTT ////////
//////////////////////////////////

void reconnect() {
  const char* clientId = "Mod-67e1f54045d20e4703ad5c4a";

  // Construct the LWT payload dynamically
  String lastWillPayload = String("{\"sensorStatus\":false,\"modId\":\"") + modId + "\"}";
  const char* lastWillPayloadString = lastWillPayload.c_str();

  // Keep running this until connected to mqtt broker
  while(!client.connected()) {
    if (client.connect(clientId, mqqt_username, mqqt_password, "mod/status/67e1f54045d20e4703ad5c4a",1, true, lastWillPayloadString)) {
      Serial.println("Broker connected");
      sendSensorStatus();
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

//////////////////////////////////
/////// SEND SENSOR STATUS ///////
//////////////////////////////////

void sendSensorStatus() {
  // Publish sensor status to message broker
  StaticJsonDocument<200> doc;

  doc["sensorStatus"] = true;
  doc["modId"] = modId;

  String requestBody;

  serializeJson(doc, requestBody);

  client.publish("mod/status/67e1f54045d20e4703ad5c4a", requestBody.c_str());
}


//////////////////////////////////
///////// SEND READINGS //////////
//////////////////////////////////

void sendReadings(int moistureReading, int temperatureReading, int humidityReading) {
  // Publish sensor readings to message broker
    if (WiFi.status() == WL_CONNECTED) {
      if (!client.connected()) {
        reconnect();
      }

      StaticJsonDocument<200> doc;

      doc["moisture"] = moistureReading;
      doc["temperature"] = temperatureReading;
      doc["humidity"] = humidityReading;
      doc["modId"] = modId;

      String requestBody;

      serializeJson(doc, requestBody);

      client.publish("mod/readings/67e1f54045d20e4703ad5c4a", requestBody.c_str());
    } else {
        Serial.println("Wifi connection failed");
    }
}


//////////////////////////////////
//////////// LOOP ////////////////
//////////////////////////////////

void loop() {
  // Read the Analog Input
  int moistureSensorValue = analogRead(33);
  int tempSensorValue = bme.temperature;
  int humiditySensorValue = bme.humidity;

  if (! bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }

  // PUBLISH TO MQTT BROKER
  sendReadings(moistureSensorValue, tempSensorValue, humiditySensorValue);

  client.loop();

  delay(timerDelay);
}