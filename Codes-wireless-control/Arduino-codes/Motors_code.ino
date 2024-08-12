#include <SPI.h>
#define step_val 4
#define pwm_max 255
#define del 100
/*arduino mega -spi pins.....
   mosi -> Pin 51
   miso -> Pin 50
   sck -> Pin 52
   ss/cs -> Pin 53
*/
//SPI pins and data definition
const int ssPin = 53;
const int packetSize = 7; // Number of data bytes (2 potentiometers)
unsigned long current_millis = 0, prev_millis = 0;
const long interval = 500;
int x_val = 0, y_val = 0, sw_drive = 0 , sw_lift = 0, sw_tilt = 0, sw_footrest = 0, sw_recline = 0;
int active_sw_sum = 0;

//motors and actuators pins definition
const int pwm_rear_l_pin = 6, pwm_rear_r_pin = 7, dir_rear_l_pin = 15, dir_rear_r_pin = 14;
const int pwm_lift_pin = 3, dir_lift_pin = 19;
const int pwm_tilt_pin = 2, dir_tilt_pin = 18;
const int pwm_footrest_pin = 5, dir_footrest_pin = 16;
const int pwm_recline_pin = 4, dir_recline_pin = 17;

//motors and actuators data definition
int pwm_rear_l = 0, pwm_rear_r = 0, dir_rear_l = 0, dir_rear_r = 0;
int duty_r = 0, duty_l = 0, dir_r = 0, dir_l = 0;
int r_duty_r = 0, r_duty_l = 0, r_dir_r = 0, r_dir_l = 0;
int signr = 1, signl = 1;
int pwm_lift = 0, dir_lift = 0;
int pwm_tilt = 0, dir_tilt = 0;
int pwm_footrest = 0, dir_footrest = 0;
int pwm_recline = 0, dir_recline = 0;


//Function for SPI Master Arduino Mega requesting SPI slave esp32 to send joystick and switches data
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


//assigning data acquired through SPI
void assignData(byte* data)
{
  x_val = data[0];
  y_val = data[1];
  sw_drive = data[2];
  sw_lift = data[3];
  sw_tilt = data[4];
  sw_footrest = data[5];
  sw_recline = data[6];
}


/////FUNCTIONS FOR MOTORS CONTROL IN DRIVE MODE////////

//FUNCTIONS FOR GETTING DUTY CYCLE OF WHEEL
void get_duty(void)
{
  if ((x_val >= 90 && x_val <= 150) && (y_val >= 90 && y_val <= 150)) //joystick neutral
  {
    Serial.print("Stop!!");
    r_duty_l = 0;
    r_duty_r = 0;
  }

  else if ((x_val >= 90 && x_val <= 150) && (y_val >= 0 && y_val <= 50)) //joystick forward
  {
    Serial.print("Forward!!");
    r_duty_l = 40;
    r_duty_r = 40;
  }

  else if ((x_val >= 90 && x_val <= 150) && (y_val >= 200 && y_val <= 255)) //joystick backward
  {
    Serial.print("Backward!!");
    r_duty_l = 40;
    r_duty_r = 40;
  }

  else if ((x_val >= 200 && x_val <= 255) && (y_val >= 90 && y_val <= 150)) //joystick RIGHTWARD
  {
    Serial.print("Right!!");
    r_duty_l = 40;
    r_duty_r = 40;
  }

  else if ((x_val >= 0 && x_val <= 50) && (y_val >= 90 && y_val <= 150)) //joystick LEFTWARD
  {
    Serial.print("Left!!");
    r_duty_l = 40;
    r_duty_r = 40;
  }

  else if ((x_val >= 200 && x_val <= 255) && (y_val >= 0 && y_val <= 50)) //joystick forward+right
  {
    Serial.print("Forward right!!");
    r_duty_l = 40;
    r_duty_r = 30;
  }

  else if ((x_val >= 0 && x_val <= 50) && (y_val >= 0 && y_val < 50)) //joystick forward+left
  {
    Serial.print("Forward left!!");
    r_duty_l = 30;
    r_duty_r = 40;
  }

  else if ((x_val >= 200 && x_val <= 255) && (y_val >= 200 && y_val <= 255)) //joystick backwrd+right
  {
    Serial.print("Backward right!!");
    r_duty_l = 40;
    r_duty_r = 30;
  }

  else if ((x_val >= 0 && x_val <= 50) && (y_val >= 200 && y_val <= 255)) //joystick backwrd+LEFT
  {
    Serial.print("Backward left!!");
    r_duty_l = 30;
    r_duty_r = 40;
  }
}

//FUNCTIONS FOR GETTING DIRECTION OF WHEEL
void get_dir(void)
{
  if (y_val >= 200 && y_val <= 255) // JOYSTICK BACKWARD ANY HOW
  {
    r_dir_r = 1;
    r_dir_l = 1;
  }
  else if ((x_val >= 200 && x_val <= 255) &&  (y_val >= 90 && y_val <= 150)) // JOYSTICK ONLY RIGHTWARD
  {
    r_dir_r = 1;
    r_dir_l = 0;
  }
  else if ((x_val >= 0 && x_val <= 50) && (y_val >= 90 && y_val <= 150)) // JOYSTICK ONLY LEFTWARD
  {
    r_dir_r = 0;
    r_dir_l = 1;
  }
  else if ((y_val >= 0) && (y_val <= 50)) // joy stick forward any how
  {
    r_dir_r = 0;
    r_dir_l = 0;
  }

}


void update_duty_l(int vall)
{

  while (duty_l != vall)
  {
    duty_l = duty_l + (signl * step_val);
    pwm_rear_l = pwm_max * (duty_l / 100.0);
    analogWrite(pwm_rear_l_pin, pwm_rear_l);
    delay(del);

  }
}

void update_duty_r(int valr)
{

  while (duty_r != valr)
  {
    duty_r = duty_r + (signr * step_val);
    pwm_rear_r = pwm_max * (duty_r / 100.0);
    analogWrite(pwm_rear_r_pin, pwm_rear_r);
    delay(del);
  }
}

void update_duty_lr(int vall, int valr)
{

  while ((duty_l != vall) && (duty_r != valr))
  {

    duty_l = duty_l + (signl * step_val);
    duty_r = duty_r + (signr * step_val);
    pwm_rear_l =  pwm_max * (duty_l / 100.0);
    pwm_rear_r = pwm_max * (duty_r / 100.0);
    analogWrite(pwm_rear_l_pin, pwm_rear_l);
    analogWrite(pwm_rear_r_pin, pwm_rear_r);

    delay(del);
  }
  if (duty_l != vall)
  {
    Serial.print("Left update!!!");
    update_duty_l(vall);

  }
  else if (duty_r != valr)
  {
    update_duty_r(valr);
    Serial.print("Right update!!!");
  }

}

void duty_dir_update(void)
{

  if (dir_r == r_dir_r && dir_l == r_dir_l)
  {
    digitalWrite(dir_rear_r_pin, r_dir_r); // REAR RIGHT MOTOR...MOTOR3
    digitalWrite(dir_rear_l_pin, r_dir_l); // REAR LEFT MOTOR....MOTOR4
    if (duty_l < r_duty_l)
    {
      signl = 1;
    }
    else if (duty_l > r_duty_l)
    {
      signl = -1;
    }

    if (duty_r < r_duty_r)
    {
      signr = 1;
    }
    else if (duty_r > r_duty_r)
    {
      signr = -1;
    }

    update_duty_lr(r_duty_l, r_duty_r);

  }

  else if (dir_r == r_dir_r && dir_l != r_dir_l) /// left direction reversed
  {
    Serial.print("Left changed!!");
    digitalWrite(dir_rear_r_pin, r_dir_r); // REAR RIGHT MOTOR...MOTOR3
    if (duty_l > 0)
    {
      signl = -1;
    }

    if (duty_r < r_duty_r)
    {
      signr = 1;
    }
    else if (duty_r > r_duty_r)
    {
      signr = -1;
    }
    update_duty_lr(0, r_duty_r);


    signl = 1;
    digitalWrite(dir_rear_l_pin, r_dir_l); // REAR LEFT MOTOR....MOTOR4
    dir_l = r_dir_l;
    update_duty_l(r_duty_l);

  }

  else if (dir_r != r_dir_r && dir_l == r_dir_l) /// right direction reversed
  {
    digitalWrite(dir_rear_l_pin, r_dir_l); // REAR LEFT MOTOR....MOTOR4

    if (duty_r > 0)
    {
      signr = -1;
    }

    if (duty_l < r_duty_l)
    {
      signl = 1;
    }
    else if (duty_l > r_duty_l)
    {
      signl = -1;
    }
    update_duty_lr(r_duty_l, 0);


    signr = 1;
    digitalWrite(dir_rear_r_pin, r_dir_r); // REAR LEFT MOTOR....MOTOR4
    dir_r = r_dir_r;
    update_duty_r(r_duty_r);

  }

  else if (dir_r != r_dir_r && dir_l != r_dir_l) /// both directions reversed
  {
    //        digitalWrite(GPIOB, GPIO_PIN_15, r_dir_l); // REAR LEFT MOTOR....MOTOR4
    //      digitalWrite(GPIOB, GPIO_PIN_14, r_dir_r); // REAR RIGHT MOTOR...MOTOR3
    if (duty_r > 0)
    {
      signr = -1;
    }

    if (duty_l > 0)
    {
      signl = -1;
    }

    update_duty_lr(0, 0);


    signr = 1;
    signl = 1;
    digitalWrite(dir_rear_l_pin, r_dir_r); // REAR LEFT MOTOR....MOTOR4
    digitalWrite(dir_rear_l_pin, r_dir_l); // REAR LEFT MOTOR....MOTOR4
    dir_r = r_dir_r;
    dir_l = r_dir_l;
    update_duty_lr(r_duty_l, r_duty_r);

  }

}

void motors_duty_dir(void)
{
  get_dir();
  get_duty();
  duty_dir_update();

}

void setup() {
  Serial.begin(115200);
  SPI.begin();
  pinMode(ssPin, OUTPUT);
  digitalWrite(ssPin, HIGH);

  //Rear Motors
  pinMode(pwm_rear_l_pin, OUTPUT);
  pinMode(dir_rear_l_pin, OUTPUT);
  pinMode(pwm_rear_r_pin, OUTPUT);
  pinMode(dir_rear_r_pin, OUTPUT);

  //Linear actuators
  pinMode(pwm_lift_pin, OUTPUT);
  pinMode(dir_lift_pin, OUTPUT);
  pinMode(pwm_tilt_pin, OUTPUT);
  pinMode(dir_tilt_pin, OUTPUT);
  pinMode(pwm_footrest_pin, OUTPUT);
  pinMode(dir_footrest_pin, OUTPUT);
  pinMode(pwm_recline_pin, OUTPUT);
  pinMode(dir_recline_pin, OUTPUT);

  // Stop the timer
  TCCR4B = 0;
  // Set fast PWM mode with non-inverting output on PWM pins 6 and 7
  TCCR4A = (1 << WGM40) | (1 << COM4A1) | (1 << COM4B1);

  // No prescaler, timer clock = system clock
  TCCR4B = (1 << WGM42) | (1 << CS40);


}

void loop() {
  byte data[packetSize];
  //  current_millis = millis();
  //  if (current_millis - prev_millis >= interval)
  //  {
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
  //    prev_millis = current_millis;

  active_sw_sum = sw_drive + sw_lift + sw_tilt + sw_footrest + sw_recline;

  // Drive mode active
  if (active_sw_sum == 1 && sw_drive == 1)
  {
    analogWrite(pwm_lift_pin, 0);
    analogWrite(pwm_tilt_pin, 0);
    analogWrite(pwm_footrest_pin, 0);
    analogWrite(pwm_recline_pin, 0);
    motors_duty_dir();
  }

  else if (active_sw_sum == 1 && sw_tilt == 1)
  {
    update_duty_lr(0, 0);
  }

  //  delay(1000); // Request every second
  //  }



}
