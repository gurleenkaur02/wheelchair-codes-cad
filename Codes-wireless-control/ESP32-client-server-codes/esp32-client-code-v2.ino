#include <WiFi.h>
#include <HTTPClient.h>
#include "driver/spi_slave.h"

const char* ssid = "ESP32-Access-Point";
const char* password = "123456789";

// Your IP address or domain name with URL path
const char* serverNameXval = "http://192.168.4.1/xval";
const char* serverNameYval = "http://192.168.4.1/yval";
const char* serverNameDrive = "http://192.168.4.1/drive";
const char* serverNameLift = "http://192.168.4.1/lift";
const char* serverNameTilt = "http://192.168.4.1/tilt";
const char* serverNameFootrest = "http://192.168.4.1/footrest";
const char* serverNameRecline = "http://192.168.4.1/recline";


String xval;
String yval;
String sw_driveval, sw_liftval, sw_tiltval, sw_frval, sw_brval;

unsigned long previousMillis = 0;
const long interval = 1000;

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  unsigned long currentMillis = millis();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi Disconnected. Attempting to reconnect...");
    WiFi.reconnect();
    unsigned long reconnectStart = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - reconnectStart) < 10000) { // 10 seconds timeout
      delay(500);
      Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("");
      Serial.print("Reconnected to WiFi network with IP Address: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println("Failed to reconnect within the timeout period.");
    }
  } else if (currentMillis - previousMillis >= interval) {
    xval = httpGETRequest(serverNameXval);
    yval = httpGETRequest(serverNameYval);
    sw_driveval = httpGETRequest(serverNameDrive);
    sw_liftval = httpGETRequest(serverNameLift);
    sw_tiltval = httpGETRequest(serverNameTilt);
    sw_frval = httpGETRequest(serverNameFootrest);
    sw_brval = httpGETRequest(serverNameRecline);
    
    Serial.print("X value: ");
    Serial.print(xval);
    Serial.print("\tY value: ");
    Serial.print(yval);
    Serial.print("\tDrive Switch: ");
    Serial.print(sw_driveval);
    Serial.print("\tLift Switch: ");
    Serial.print(sw_liftval);
    Serial.print("\tTilt Switch: ");
    Serial.print(sw_tiltval);
    Serial.print("\tFootrest Switch: ");
    Serial.print(sw_frval);
    Serial.print("\tRecline Switch: ");
    Serial.print(sw_brval);
    Serial.println("");

    previousMillis = currentMillis;
  }
}

String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;

  http.begin(client, serverName);

  int httpResponseCode = http.GET();

  String payload = "--";

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
    Serial.print("Response payload: ");
    Serial.println(payload);
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();

  return payload;
}
