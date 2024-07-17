#include <SPI.h>

/*arduino mega -spi pins.....
   mosi -> Pin 51
   miso -> Pin 50
   sck -> Pin 52
   ss/cs -> Pin 53
*/

const int ssPin = 53;
const int packetSize = 7; // Number of data bytes (2 potentiometers)
unsigned long current_millis = 0, prev_millis = 0;
const long interval = 500;

void requestSensorData(byte* data, int length) {

  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0)); // Set SPI clock to 1 MHz
  digitalWrite(ssPin, LOW); // Select the slave
  //  SPI.transfer(0x01); // Send request command

  for (int i = 0; i < length; i++) {
    data[i] = SPI.transfer(0x00); // Read the response
  }
  digitalWrite(ssPin, HIGH); // Deselect the slave
  SPI.endTransaction();
}


void setup() {
  Serial.begin(115200);
  SPI.begin();
  pinMode(ssPin, OUTPUT);
  digitalWrite(ssPin, HIGH);
}

void loop() {
  byte data[packetSize];
  current_millis = millis();
  if (current_millis - prev_millis >= interval)
  {
    requestSensorData(data, packetSize);

    Serial.print("X Value: ");
    Serial.print(data[0]);
    Serial.print("\t");
    Serial.print("Y Value: ");
    Serial.print(data[1]);
    Serial.print("\t");
    Serial.print("Drive Switch State: ");
    Serial.print(data[2]);
    Serial.print("\t");
    Serial.print("Lift Switch State: ");
    Serial.print(data[3]);
    Serial.print("\t");
    Serial.print("Tilt Switch State: ");
    Serial.print(data[4]);
    Serial.print("\t");
    Serial.print("Footrest Switch State: ");
    Serial.print(data[5]);
    Serial.print("\t");
    Serial.print("Recline Switch State: ");
    Serial.println(data[6]);
    prev_millis = current_millis;

    //  delay(1000); // Request every second
  }

}
