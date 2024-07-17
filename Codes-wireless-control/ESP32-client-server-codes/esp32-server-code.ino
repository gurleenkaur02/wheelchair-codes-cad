/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-client-server-wi-fi/

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/


//192.168.4.1



// Import required libraries
#include "WiFi.h"
#include "ESPAsyncWebServer.h"

#define xpin 34
#define ypin 35
#define sw_drive 32
#define sw_lift 33
#define sw_tilt 25
#define sw_footrest 26
#define sw_recline 27



// Set your access point network credentials
const char* ssid = "ESP32-Access-Point";
const char* password = "123456789";



// Create AsyncWebServer object on port 80
AsyncWebServer server(80);



void setup() {
  // Serial port for debugging purposes
  pinMode(sw_drive, INPUT);
  pinMode(sw_lift, INPUT);
  pinMode(sw_tilt, INPUT);
  pinMode(sw_footrest, INPUT);
  pinMode(sw_recline, INPUT);

  Serial.begin(115200);
  Serial.println();

  // Setting the ESP as an access point
  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/xval", HTTP_GET, [](AsyncWebServerRequest * request) {
    int xval = (analogRead(xpin)) / 16;
    request->send_P(200, "text/plain", String(xval).c_str());
    Serial.print("X Value:");
    Serial.println(xval);
  });
  server.on("/yval", HTTP_GET, [](AsyncWebServerRequest * request) {
    int yval = (analogRead(ypin)) / 16;
    request->send_P(200, "text/plain", String(yval).c_str());
    Serial.print("Y Value:");
    Serial.println(yval);
  });

  server.on("/drive", HTTP_GET, [](AsyncWebServerRequest * request) {
    int sw_drive_val = digitalRead(sw_drive);
    request->send_P(200, "text/plain", String(sw_drive_val).c_str());
    Serial.print("Drive Value:");
    Serial.println(sw_drive_val);
  });
  server.on("/lift", HTTP_GET, [](AsyncWebServerRequest * request) {
    int sw_lift_val = digitalRead(sw_lift);
    request->send_P(200, "text/plain", String(sw_lift_val).c_str());
    Serial.print("Lift Value:");
    Serial.println(sw_lift_val);
  });
  server.on("/tilt", HTTP_GET, [](AsyncWebServerRequest * request) {
    int sw_tilt_val = digitalRead(sw_tilt);
    request->send_P(200, "text/plain", String(sw_tilt_val).c_str());
    Serial.print("Tilt Value:");
    Serial.println(sw_tilt_val);
  });

  server.on("/footrest", HTTP_GET, [](AsyncWebServerRequest * request) {
    int sw_footrest_val = digitalRead(sw_footrest);
    request->send_P(200, "text/plain", String(sw_footrest_val).c_str());
    Serial.print("Footrest value");
    Serial.println(sw_footrest_val);
  });

  server.on("/recline", HTTP_GET, [](AsyncWebServerRequest * request) {
    int sw_recline_val = digitalRead(sw_recline);
    request->send_P(200, "text/plain", String(sw_recline_val).c_str());
    Serial.print("Recline Value:");
    Serial.println(sw_recline_val);
  });

  // Start server
  server.begin();

}

void loop() {

}
