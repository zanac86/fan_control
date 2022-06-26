#define FAN_PWM_PIN 3

uint8_t ramp_x = 0;

#define PERIOD 250
uint32_t timer = 0;

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(FAN_PWM_PIN, OUTPUT);

  TCCR2A = 0x23;     // COM2B1, WGM21, WGM20
  TCCR2B = 0x0A;   // WGM21, Prescaler = /8
  OCR2A = 79;    // TOP DO NOT CHANGE, SETS PWM PULSE RATE
  OCR2B = 79;    // duty cycle for Pin 3 (0-79) generates 1 500nS pulse even when 0 :(
  OCR2B = 39;    // duty cycle for Pin 3 (0-79) generates 1 500nS pulse even when 0 :(

}

void loop() {

  if (millis() - timer >= PERIOD) {
    timer += PERIOD;
    OCR2B = ramp_x;
    ramp_x = (ramp_x + 1) % 80;
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  }

}
