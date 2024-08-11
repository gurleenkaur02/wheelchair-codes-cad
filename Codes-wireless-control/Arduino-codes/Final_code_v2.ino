#include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

#define step_val 5
#define pwm_max 255
#define del 100

const int ssPin = 53;
const int packetSize = 7;  // Number of data bytes (2 potentiometers)
byte data[packetSize];
unsigned long current_millis = 0, prev_millis = 0;
const long interval = 500;
int x_val = 0, y_val = 0, sw_drive = 0, sw_lift = 0, sw_tilt = 0, sw_footrest = 0, sw_recline = 0;
int active_sw_sum = 0;

const int pwm_rear_l_pin = 6, pwm_rear_r_pin = 7, dir_rear_l_pin = 15, dir_rear_r_pin = 14;
const int pwm_lift_pin = 3, dir_lift_pin = 19;
const int pwm_tilt_pin = 2, dir_tilt_pin = 18;
const int pwm_footrest_pin = 5, dir_footrest_pin = 16;
const int pwm_recline_pin = 4, dir_recline_pin = 17;
const int asc_limit_sw = A15, desc_limit_sw = A14;

int pwm_rear_l = 0, pwm_rear_r = 0, dir_rear_l = 0, dir_rear_r = 0;
int duty_r = 0, duty_l = 0, dir_r = 0, dir_l = 0;
int r_duty_r = 0, r_duty_l = 0, r_dir_r = 0, r_dir_l = 0;
int signr = 1, signl = 1;
int pwm_lift = 0, dir_lift = 0;
int pwm_tilt = 0, dir_tilt = 0;
int pwm_footrest = 0, dir_footrest = 0;
int pwm_recline = 0, dir_recline = 0;

Adafruit_BNO055 bno = Adafruit_BNO055(55);
float theta;
float theta_ref = 1.5;  // Defining reference angle in degrees
int start = 0;

int lift_dir = 0, tilt_dir = 0;
int lift_pwm = 0, tilt_pwm = 0;

float k_p = -100.0, k_i = 0.0, k_d = -30.0;
unsigned long lastTime = 0;
unsigned long now = 0;
float error;
float last_error = 0.0;
float dt;
float error_sum = 0.0;
float delta_error;
float out_pwm;

void requestSensorData(byte* data, int length) {
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));  // Set SPI clock to 1 MHz
  digitalWrite(ssPin, LOW);                                         // Select the slave

  for (int i = 0; i < length; i++) {
    data[i] = SPI.transfer(0x00);  // Read the response
  }

  digitalWrite(ssPin, HIGH);  // Deselect the slave
  SPI.endTransaction();
}

void assignData(byte* data) {
  x_val = data[0];
  y_val = data[1];
  sw_drive = data[2];
  sw_lift = data[3];
  sw_tilt = data[4];
  sw_footrest = data[5];
  sw_recline = data[6];
}

void setup() {
  Serial.begin(115200);
  SPI.begin();
  pinMode(ssPin, OUTPUT);
  digitalWrite(ssPin, HIGH);

  // Initialize motor pins
  pinMode(pwm_rear_l_pin, OUTPUT);
  pinMode(pwm_rear_r_pin, OUTPUT);
  pinMode(dir_rear_l_pin, OUTPUT);
  pinMode(dir_rear_r_pin, OUTPUT);

  // Initialize lift actuator pins
  pinMode(pwm_lift_pin, OUTPUT);
  pinMode(dir_lift_pin, OUTPUT);

  // Initialize tilt actuator pins
  pinMode(pwm_tilt_pin, OUTPUT);
  pinMode(dir_tilt_pin, OUTPUT);

  // Initialize footrest actuator pins
  pinMode(pwm_footrest_pin, OUTPUT);
  pinMode(dir_footrest_pin, OUTPUT);

  // Initialize recline actuator pins
  pinMode(pwm_recline_pin, OUTPUT);
  pinMode(dir_recline_pin, OUTPUT);

  // Initialize limit switch
  pinMode(asc_limit_sw, INPUT);

  // Initialize BNO
  Serial.println("Orientation Sensor Test");
  Serial.println("");
  if (!bno.begin()) {
    Serial.print("Oops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while (1);
  }
  delay(1000);
  bno.setExtCrystalUse(true);
  delay(2000);
}

void loop() {
  requestSensorData(data, packetSize);
  assignData(data);

  Serial.print("X Value: ");
  Serial.print(x_val);
  Serial.print("\t");
  Serial.print("Y Value: ");
  Serial.print(y_val);
  Serial.print("\t");
  Serial.print("Drive Switch State: ");
  Serial.print(sw_drive);
  Serial.print("\t");
  Serial.print("Lift Switch State: ");
  Serial.print(sw_lift);
  Serial.print("\t");
  Serial.print("Tilt Switch State: ");
  Serial.print(sw_tilt);
  Serial.print("\t");
  Serial.print("Footrest Switch State: ");
  Serial.print(sw_footrest);
  Serial.print("\t");
  Serial.print("Recline Switch State: ");
  Serial.println(sw_recline);

  if (sw_drive == 1) {  // Drive mode
    analogWrite(pwm_lift_pin, 0);
    analogWrite(pwm_tilt_pin, 0);
    analogWrite(pwm_footrest_pin, 0);
    analogWrite(pwm_recline_pin, 0);

    if (y_val >= 0 && y_val <= 20 && x_val >= 100 && x_val <= 130) {
      Serial.println("Forward");
      analogWrite(pwm_rear_l_pin, 128);
      analogWrite(pwm_rear_r_pin, 128);
      digitalWrite(dir_rear_l_pin, 0);
      digitalWrite(dir_rear_r_pin, 0);
    } else if (y_val >= 235 && y_val <= 255 && x_val >= 100 && x_val <= 130) {
      Serial.println("Backward");
      analogWrite(pwm_rear_l_pin, 128);
      analogWrite(pwm_rear_r_pin, 128);
      digitalWrite(dir_rear_l_pin, 1);
      digitalWrite(dir_rear_r_pin, 1);
    } else if (y_val >= 100 && y_val <= 130 && x_val >= 235 && x_val <= 255) {
      Serial.println("Right");
      analogWrite(pwm_rear_l_pin, 128);
      analogWrite(pwm_rear_r_pin, 128);
      digitalWrite(dir_rear_l_pin, 0);
      digitalWrite(dir_rear_r_pin, 1);
    } else if (y_val >= 100 && y_val <= 130 && x_val >= 0 && x_val <= 20) {
      Serial.println("Left");
      analogWrite(pwm_rear_l_pin, 128);
      analogWrite(pwm_rear_r_pin, 128);
      digitalWrite(dir_rear_l_pin, 1);
      digitalWrite(dir_rear_r_pin, 0);
    } else {
      analogWrite(pwm_rear_l_pin, 0);
      analogWrite(pwm_rear_r_pin, 0);
    }

  } else if (sw_lift == 1) {  // Lifting Mode
    Serial.println("Lifting Function");
    // Turn off other features
    analogWrite(pwm_rear_l_pin, 0);
    analogWrite(pwm_rear_r_pin, 0);
    analogWrite(pwm_footrest_pin, 0);
    analogWrite(pwm_recline_pin, 0);

    if (y_val >= 0 && y_val < 20) {
      digitalWrite(dir_tilt_pin, LOW);
      analogWrite(pwm_tilt_pin, 128);
      Serial.println("Tilt forward");
    } else if (y_val > 235 && y_val <= 255) {
      digitalWrite(dir_tilt_pin, HIGH);
      analogWrite(pwm_tilt_pin, 128);
      Serial.println("Tilt backward");
    } else {
      analogWrite(pwm_tilt_pin, 0);
    }
    // Acquire angle data from sensor
    sensors_event_t event;
    bno.getEvent(&event);
    theta = event.orientation.y;  // Acquire angle in y direction w.r.t sensor
    error = theta_ref - theta;    // Calculate error

    now = millis();
    dt = (now - lastTime) / 1000.0;
    lastTime = now;
    error_sum += error * dt;                                      // Add error value iteratively in each loop to compute integral
    delta_error = (error - last_error) / dt;                      // Compute error difference iteratively to find error derivative
    out_pwm = k_p * error + k_i * error_sum + k_d * delta_error;  // Compute PWM output by adding P I and D terms
    last_error = error;

    if (((out_pwm > (-30) && out_pwm <= 0) || ((out_pwm < 30 && out_pwm >= 0))))  // pwm=0 for lift actuator dead zone
    {
      lift_pwm = 0;
      Serial.println("STOP!");
    } else if (out_pwm <= (-30) && (digitalRead(desc_limit_sw) == LOW))  // retraction of lift actuator
    {
      lift_pwm = int(constrain(abs(out_pwm), 30, 255));
      lift_dir = 1;
      Serial.print("retract\t");
      Serial.println(lift_pwm);
    } else if (out_pwm >= 30 && (digitalRead(asc_limit_sw) == LOW))  // extension of lift actuator
    {
      lift_pwm = int(constrain(abs(out_pwm), 30, 255));
      lift_dir = 0;
      Serial.println("extend");
    } else if (digitalRead(asc_limit_sw) == HIGH) {
      Serial.println("STOP! limit switch actuated");
      lift_pwm = 0;
      tilt_pwm = 0;
    }

    analogWrite(pwm_lift_pin, lift_pwm);
    digitalWrite(dir_lift_pin, lift_dir);

    Serial.print(lift_pwm);
    Serial.print("\t");
    Serial.print(lift_dir);
    Serial.println("\n");
  } else if (sw_tilt == 1) {  // Tilting Mode
    analogWrite(pwm_rear_l_pin, 0);
    analogWrite(pwm_rear_r_pin, 0);
    analogWrite(pwm_footrest_pin, 0);
    analogWrite(pwm_recline_pin, 0);
    analogWrite(pwm_lift_pin, 0);

    if (y_val >= 0 && y_val < 20) {
      analogWrite(pwm_tilt_pin, 128);
      digitalWrite(dir_tilt_pin, 0);
      Serial.println("Extending Tilt");
    } else if (y_val > 235 && y_val <= 255) {
      analogWrite(pwm_tilt_pin, 128);
      digitalWrite(dir_tilt_pin, 1);
      Serial.println("Retracting tilt");
    } else {
      analogWrite(pwm_tilt_pin, 0);
    }
  } else if (sw_recline == 1) {  // Recline Mode
    analogWrite(pwm_rear_l_pin, 0);
    analogWrite(pwm_rear_r_pin, 0);
    analogWrite(pwm_footrest_pin, 0);
    analogWrite(pwm_lift_pin, 0);
    analogWrite(pwm_tilt_pin, 0);

    if (y_val >= 0 && y_val < 20) {
      analogWrite(pwm_recline_pin, 128);
      digitalWrite(dir_recline_pin, 0);
      Serial.println("Extending Recliner");
    } else if (y_val > 235 && y_val <= 255) {
      analogWrite(pwm_recline_pin, 128);
      digitalWrite(dir_recline_pin, 1);
      Serial.println("Retracting recliner");
    } else {
      analogWrite(pwm_recline_pin, 0);
    }
  } else if (sw_footrest == 1) {  // Footrest inclination mode
    analogWrite(pwm_rear_l_pin, 0);
    analogWrite(pwm_rear_r_pin, 0);
    analogWrite(pwm_recline_pin, 0);
    analogWrite(pwm_lift_pin, 0);
    analogWrite(pwm_tilt_pin, 0);

    if (y_val >= 0 && y_val < 20) {
      analogWrite(pwm_footrest_pin, 128);
      digitalWrite(dir_footrest_pin, 0);
      Serial.println("Extending Footrest");
    } else if (y_val > 235 && y_val <= 255) {
      digitalWrite(dir_footrest_pin, 1);
      analogWrite(pwm_footrest_pin, 128);
      Serial.println("Retracting Footrest");
    } else {
      analogWrite(pwm_footrest_pin, 0);
    }
  } else {                              //none active 
    analogWrite(pwm_rear_l_pin, 0);
    analogWrite(pwm_rear_r_pin, 0);
    analogWrite(pwm_footrest_pin, 0);
    analogWrite(pwm_recline_pin, 0);
    analogWrite(pwm_lift_pin, 0);
    analogWrite(pwm_tilt_pin, 0);
  }

  delay(250);
}
