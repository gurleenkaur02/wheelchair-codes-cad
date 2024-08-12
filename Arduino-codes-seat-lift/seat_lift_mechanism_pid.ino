  #include <Wire.h>
  #include <Adafruit_Sensor.h>
  #include <Adafruit_BNO055.h>
  #include <utility/imumaths.h>

#define tilt_pwm_pin 6
#define tilt_dir_pin 7
#define lift_dir_pin 8
#define lift_pwm_pin 9
#define asc_limit_sw 3
#define desc_limit_sw 4
#define joystick_Y A0


Adafruit_BNO055 bno = Adafruit_BNO055(55);
float theta_z;
float theta_ref = 1.5;
int start = 0;

int lift_dir = 0, tilt_dir = 0;
int lift_pwm = 0, tilt_pwm = 0;
int asc_sw_state = 0, desc_sw_state = 0;

float k_p = -100.0, k_i = 0.0, k_d = -30.0;
unsigned long lastTime = 0;
unsigned long now = 0;
float error_z;
float last_error_z = 0.0;
float dt;
float error_sum_z = 0.0;
float delta_error_z;
float out_pwm;

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  pinMode(tilt_pwm_pin, OUTPUT);
  pinMode(tilt_dir_pin, OUTPUT);
  pinMode(lift_dir_pin, OUTPUT);
  pinMode(lift_pwm_pin, OUTPUT);
  pinMode(asc_limit_sw, INPUT);
  pinMode(desc_limit_sw, INPUT);
  //analogWrite(lift_pwm_pin,128);
  //digitalWrite(lift_dir_pin,HIGH);
  //delay(1000);

  Serial.println("Orientation Sensor Test"); Serial.println("");

  /* Initialise the sensor */
  if (!bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while (1);
  }

  delay(1000);
  bno.setExtCrystalUse(true);
  delay(2000);
}

void loop() {

  Serial.println(analogRead(joystick_Y));
  if (analogRead(joystick_Y) <= 200)
  {
    digitalWrite(tilt_dir_pin, LOW);
    analogWrite(tilt_pwm_pin, 128);
    Serial.println("Tilt forward");
  }
  else if (analogRead(joystick_Y) >= 800)
  {
    digitalWrite(tilt_dir_pin, HIGH);
    analogWrite(tilt_pwm_pin, 128);
    Serial.println("Tilt backward");
  }
  else
  {
    analogWrite(tilt_pwm_pin, 0);
  }

  sensors_event_t event;
  bno.getEvent(&event);
  theta_z = event.orientation.z;
  if (theta_z < 0)
  {
    theta_z = -theta_z - 180.0;
  }
  else
  {
    theta_z = 180.0 - theta_z;
  }



  error_z = theta_ref - theta_z;
  Serial.println((-1) * (error_z));

  if (start == 0)
  {
    out_pwm = k_p * error_z;
    start = 1;
    now = millis();
    lastTime = now;

  }
  else
  {
    now = millis();
    dt = (now - lastTime) / 1000.0;
    lastTime = now;
    error_sum_z += error_z * dt;
    delta_error_z = (error_z - last_error_z) / dt;
    out_pwm = k_p * error_z + k_d * delta_error_z;

  }
  last_error_z = error_z;
  Serial.println(dt);
  if (((out_pwm > (-30) && out_pwm <= 0) || ((out_pwm < 30 && out_pwm >= 0 )))) //pwm=0 for lift actuator dead zone
  {
    lift_pwm = 0;
    Serial.println("STOP!");
  }
  else if (out_pwm <= (-30) && (digitalRead(desc_limit_sw) == LOW))  //retraction of lift actuator
  {
    lift_pwm = int(constrain(abs(out_pwm), 30, 255));
    lift_dir = 1;
    Serial.print("retract\t");
    Serial.println(lift_pwm);
  }
  else if (out_pwm >= 30 && (digitalRead(asc_limit_sw) == LOW))    //extension of lift actuator
  {
    lift_pwm = int(constrain(abs(out_pwm), 30, 255));
    lift_dir = 0;
    Serial.println("extend");
  }
  else
  {
    Serial.println("STOP! limit switch actuated");
    lift_pwm = 0;
  }

  analogWrite(lift_pwm_pin, lift_pwm);
  digitalWrite(lift_dir_pin, lift_dir);

  Serial.print(lift_pwm);
  Serial.print("\t");
  Serial.print(lift_dir);
  Serial.println("\n");

}
