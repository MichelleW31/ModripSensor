// LIBRARIES
#include <PubSubClient.h>

#include <IRremote.hpp>

#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Wire.h>
#include "Adafruit_BME680.h"

#include "secrets.h"

// MQTT SERVER
const char* mqtt_server ="902e1d0dba3944fa88c5f6caac765b57.s1.eu.hivemq.cloud"

// SERVER CONNECTION
const char* modId = "67e1f54045d20e4703ad5c4a";
// Will need to make the modId dynamic in url
const char* serverConnectionUrl = "https://modripservice.onrender.com/mods/67e1f54045d20e4703ad5c4a";
const char* authEndpointUrl = "https://modripservice.onrender.com/authenticate_mod";
const char* sensorStatusUrl = "https://modripservice.onrender.com/sensor_status/67e1f54045d20e4703ad5c4a";

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

// Set timer to 10 minutes
int timerDelay = 600000;
// Set timer to 7 minutes
// int timerDelay = 420000;
// Set timer to 5 minutes
// int timerDelay = 300000;
// Set timer to 1 minutes
// int timerDelay = 60000;

//////////////////////////////////
//////////// SETUP ///////////////
//////////////////////////////////

void setup() {
  
  // Setup Serial Monitor
  Serial.begin(9600);

  // Connect SCL and SDA pins
  Wire.begin(33, 32);

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

  Serial.println("Connecting");

  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to WiFi network");

  idToken = getIdToken(modId);
}

//////////////////////////////////
//////////// LOOP ////////////////
//////////////////////////////////

void loop() {
  // Read the Analog Input
  int moistureSensorValue = analogRead(35);

  if (! bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }

  int tempSensorValue = bme.temperature;
  int humiditySensorValue = bme.humidity;
  
  // PUT CALL
  if (idToken != "") {
    sendReadings(moistureSensorValue, tempSensorValue, humiditySensorValue, idToken);
  }
  
  delay(timerDelay);
}

//////////////////////////////////
///////// GET ID TOKEN ///////////
//////////////////////////////////

String getIdToken (String modId) {
  HTTPClient http;
  http.begin(authEndpointUrl);
  http.addHeader("Content-Type", "application/json; charset=utf-8");

  StaticJsonDocument<200> doc;

  doc["modId"] = modId;

  String requestBody;

  serializeJson(doc,requestBody);

  int httpResponseCode = http.POST(requestBody);

  if (httpResponseCode == 200) {
        String response = http.getString();
        Serial.println("Authentication successful");

        DynamicJsonDocument doc(1024);
        deserializeJson(doc, response);

        return doc["idToken"].as<String>();  // Extract ID Token
    } else {
        Serial.println("Failed to get ID Token: " + http.errorToString(httpResponseCode));
        return "";
    }

    http.end();
}

//////////////////////////////////
////// SEND SENSOR STATUS ////////
//////////////////////////////////

void sendSensorStatus() {
  HTTPClient http;
  http.begin(sensorStatusUrl);

  // Set up request
  http.addHeader("Content-Type", "application/json; charset=utf-8");
  http.addHeader("Content-Length", "0");
  // http.addHeader("Authorization", "Bearer " + idToken);

  http.setTimeout(10000);

  int httpResponseCode = http.POST("{}");

  if (httpResponseCode == 200) {
    Serial.println("Sensor status updated");
  } else {
    Serial.println(httpResponseCode);
    Serial.println("Failed to update sensor status: " + http.errorToString(httpResponseCode));
  }

    http.end();
}


//////////////////////////////////
///////// SEND READINGS //////////
//////////////////////////////////

void sendReadings(int moistureReading, int temperatureReading, int humidityReading, String idToken) {
  // Make api call to send sensor readings
    if (WiFi.status() == WL_CONNECTED) {
      // HTTPClient http;

      // http.begin(serverConnectionUrl);

      // // Set up request
      // http.addHeader("Content-Type", "application/json; charset=utf-8");
      // http.addHeader("Authorization", "Bearer " + idToken);

      // StaticJsonDocument<200> doc;

      // doc["moisture"] = moistureReading;
      // doc["temperature"] = temperatureReading;
      // doc["humidity"] = humidityReading;

      // String requestBody;

      // serializeJson(doc, requestBody);

      // // Make PUT request
      // int httpCode = http.PUT(requestBody);

      // if (httpCode == 200) {
      //   String payload = http.getString();

      //   Serial.println(httpCode);

      //   Serial.println(payload);

        sendSensorStatus();
      } else if (httpCode == 403) { 
            Serial.println("ID Token expired. Getting a new one...");

            String newIdToken = getIdToken(modId);

            if (newIdToken != "") {
                Serial.println("Retrying sensor data update...");

                sendReadings(moistureReading, temperatureReading, humidityReading, newIdToken);  
            } else {
                Serial.println("Failed to refresh ID Token. Update not sent.");
            }
      }
      else {       
        Serial.println("HTTP Request failed, error: " +  http.getString());
      }

      http.end();
    } else {
        Serial.println("Wifi connection failed");
    }
}


