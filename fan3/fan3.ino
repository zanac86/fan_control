/*
   Arduino pro mini
   регулятор скорости оборотов кулера с pwm.
   Частота ШИМ 25 КГц.
   Обороты кулера зависят от ширины импульса.
   Для выхода на pin 9 - ШИМ можно задавать 0..320
   При 320 - будет просто 1 на выходе

   На pin 11 - выключатель
*/

/*
   Project pwm coontroller for 4 fans
   https://github.com/abelgomez/pwm-fan-controller
*/

#define BUTTON_PIN 11
#define PWM_PIN     9

unsigned int pwm_max = 320;
unsigned int pwm_old = 0;

unsigned int get_new_pwm(uint8_t level)
{
  if (level == 1)
  {
    // 1 - значит выключатель разомкнут - внутри pull-up
    return pwm_max / 2;
  }
  return pwm_max;
}

void set_pwm_pin9()
{
  //Set PWM frequency to about 25khz on pins 9,10 (timer 1 mode 10, no prescale, count to 320)
  TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM11);
  TCCR1B = (1 << CS10) | (1 << WGM13);
  ICR1 = 320;
  OCR1A = 320;
  pinMode(PWM_PIN, OUTPUT);
  OCR1A = 320;
}

void setup() {
  Serial.begin(9600);
  delay(100);
  // вход выключателя к +, замыкать на gnd
  //  pinMode(BUTTON_PIN, INPUT_PULLUP); // для обычного выключателя или кнопки
  pinMode(BUTTON_PIN, INPUT); // для ttp223
  set_pwm_pin9();
}

void loop() {
  digitalWrite(13, HIGH);
  delay(pwm_old);
  uint8_t button_state = digitalRead(BUTTON_PIN); // 1 - значит выключатель разомкнут - внутри pull-up
  unsigned int pwm_x = get_new_pwm(button_state);
  if (pwm_x != pwm_old)
  {
    OCR1A = pwm_x; // pin 9
    pwm_old = pwm_x;
    Serial.print("New pwm\n");
    Serial.print(pwm_x);
    Serial.print("\n");
  }
  digitalWrite(13, LOW);
  delay(pwm_old);
}
