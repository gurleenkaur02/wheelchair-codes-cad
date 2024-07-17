#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include "driver/spi_slave.h"

const char* ssid = "ESP32-Access-Point";
const char* password = "123456789";


const int ssPin = 5;
const int data_size = 7;
byte sensorData[data_size];

// Your IP address or domain name with URL path
const char* serverNameXval = "http://192.168.4.1/xval";
const char* serverNameYval = "http://192.168.4.1/yval";
const char* serverNameDrive = "http://192.168.4.1/drive";
const char* serverNameLift = "http://192.168.4.1/lift";
const char* serverNameTilt = "http://192.168.4.1/tilt";
const char* serverNameRecline = "http://192.168.4.1/recline";
const char* serverNameFootrest = "http://192.168.4.1/footrest";

String xval, yval;
String sw_drive_val, sw_lift_val, sw_tilt_val, sw_recline_val, sw_footrest_val;


void setup() {
  Serial.begin(115200);
  pinMode(ssPin, INPUT);


  // SPI bus configuration
  spi_bus_config_t buscfg = {};
  buscfg.mosi_io_num = 23;
  buscfg.miso_io_num = 19;
  buscfg.sclk_io_num = 18;
  buscfg.quadwp_io_num = -1;
  buscfg.quadhd_io_num = -1;

  // SPI slave configuration
  spi_slave_interface_config_t slvcfg = {};
  slvcfg.spics_io_num = ssPin;
  slvcfg.flags = 0;
  slvcfg.queue_size = 3;
  slvcfg.mode = 0;
  slvcfg.post_setup_cb = NULL;
  slvcfg.post_trans_cb = NULL;

  // Initialize SPI bus as slave
  esp_err_t ret = spi_slave_initialize(VSPI_HOST, &buscfg, &slvcfg, 1);
  if (ret != ESP_OK) {
    Serial.println("Failed to initialize SPI slave");
    while (1);
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to Wi-Fi");

  // Initialize sensor data
  readSensors();
}

void loop() {

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
  }

  else {
    xval = httpGETRequest(serverNameXval);
    yval = httpGETRequest(serverNameYval);
    sw_drive_val = httpGETRequest(serverNameDrive);
    sw_lift_val = httpGETRequest(serverNameLift);
    sw_tilt_val = httpGETRequest(serverNameTilt);
    sw_footrest_val = httpGETRequest(serverNameFootrest);
    sw_recline_val = httpGETRequest(serverNameRecline);

    Serial.print("X value: ");
    Serial.print(xval);
    Serial.print("\tY value: ");
    Serial.print(yval);
    Serial.print("\tDrive Switch value: ");
    Serial.println(sw_drive_val);
    sensorData[0] = xval.toInt();              // x value joystick
    sensorData[1] = yval.toInt();              // y value joystick
    sensorData[2] = sw_drive_val.toInt();      // drive value switch
    sensorData[3] = sw_lift_val.toInt();       ///lift value switch
    sensorData[4] = sw_tilt_val.toInt();       //tilt value switch
    sensorData[5] = sw_footrest_val.toInt();   //footrest value switch
    sensorData[6] = sw_recline_val.toInt();    //recline value switch
  }

  //spi slave transaction
  spi_slave_transaction_t t;
  memset(&t, 0, sizeof(t));

  t.length = sizeof(sensorData) * 8; // length in bits
  t.tx_buffer = sensorData;
  printTxBuffer(t.tx_buffer, sizeof(sensorData));
  esp_err_t ret = spi_slave_transmit(VSPI_HOST, &t, portMAX_DELAY);
  if (ret != ESP_OK) {
    Serial.println("Failed to transmit SPI data");
  }

}


void printTxBuffer(const void* buffer, size_t length) {
  const byte* byteBuffer = (const byte*)buffer;
  Serial.print("t.tx_buffer contents: ");
  for (size_t i = 0; i < length; i++) {
    Serial.print(byteBuffer[i]);
    Serial.print(" ");
  }
  Serial.println("\n");
}

void readSensors() {
  // Initialize with dummy values or read actual values if needed

  for (int i = 0; i < data_size; i++)
  {
    sensorData[i] = 0;
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
