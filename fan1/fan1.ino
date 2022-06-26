#include "GyverButton.h"

/*
регулятор скорости оборотов кулера с 4 проводами
на arduino nano или
сенсорная ttp кнопка на pin 2
pwm выход на pin 3
*/


#define BUTTON_PIN 2
GButton touch(BUTTON_PIN, LOW_PULL, NORM_OPEN);

// 100% - 0, 50% - 1, ramp - 2
uint8_t fanMode = 0;
bool newMode = false;
unsigned long change_time = 300;
unsigned long last_change = 0;
unsigned int ramp_x = 0;

void setup() {
  Serial.begin(115200);
  // generate 25kHz PWM pulse rate on Pin 3
  const int PWMPin = 3;
  pinMode(PWMPin, OUTPUT);   // OCR2B sets duty cycle
  // Set up Fast PWM on Pin 3
  TCCR2A = 0x23;     // COM2B1, WGM21, WGM20
  // Set prescaler
  TCCR2B = 0x0A;   // WGM21, Prescaler = /8
  // Set TOP and initialize duty cycle to zero(0)
  OCR2A = 79;    // TOP DO NOT CHANGE, SETS PWM PULSE RATE
  OCR2B = 79;    // duty cycle for Pin 3 (0-79) generates 1 500nS pulse even when 0 :(
}

void checkMode()
{
  touch.tick();
  if (touch.hasClicks())
  {
    if (touch.getClicks() == 1)
    {
      fanMode = (fanMode + 1) % 3;
      newMode = true;
    }
  }
}

void loop() {
  checkMode();
  unsigned int x = 0;
  if (millis() - last_change > change_time)
  {
    change_time = 300;
    last_change = millis();
    ramp_x = (ramp_x + 1) % 80;
    if (fanMode == 2)
    {
      newMode = true;
    }
  }

  switch (fanMode)
  {
    case 0:
      x = 79;
      break;
    case 1:
      x = 50;
      break;
    case 2:
      x = ramp_x;
      break;
  }


  if (newMode)
  {
    Serial.print(fanMode);
    Serial.print("\n");
    Serial.print(x);
    Serial.print("\n");
    OCR2B = x;
    newMode = false;
  }

}
