/*
   Регулятор скорости оборотов кулера с pwm.
   Arduino pro mini
   Atmega 328 5V 16Mhz
   Частота ШИМ 25 КГц.
   Обороты кулера зависят от ширины импульса.
   Для выхода на pin 9 - ШИМ можно задавать 0..320
   При 320 - будет просто 1 на выходе - кулер на макс

   На pin 11 - выключатель
*/

/*
   Project pwm coontroller for 4 fans
   https://github.com/abelgomez/pwm-fan-controller
*/
#include <EEPROM.h>
int addr_current_mode_num = 100;


#define EB_NO_FOR           // отключить поддержку pressFor/holdFor/stepFor и счётчик степов (экономит 2 байта оперативки)
#define EB_NO_CALLBACK      // отключить обработчик событий attach (экономит 2 байта оперативки)
#define EB_NO_COUNTER       // отключить счётчик энкодера (экономит 4 байта оперативки)
#define EB_NO_BUFFER        // отключить буферизацию энкодера (экономит 1 байт оперативки)
#include "EncButton.h"

#define BUTTON_PIN 11
#define PWM_PIN 9

Button button(BUTTON_PIN, INPUT, HIGH);

uint16_t pwm_max = 320;
uint16_t pwm_old = 0;

enum FAN_MODE
{
    MODE_MAX_SPEED,       // 0
    MODE_MID_SPEED,       // 190
    MODE_LOW_SPEED,       // 320
    MODE_SWITCH_MIN_MAX,  // 30s LOW, 30s MAX,
    MODE_SMOOTH_MIN_MAX,  // 30s LOW->MAX, 30s MAX-LOW
};

FAN_MODE modes[] = { MODE_MAX_SPEED, MODE_MID_SPEED, MODE_LOW_SPEED, MODE_SWITCH_MIN_MAX, MODE_SMOOTH_MIN_MAX };
uint8_t modes_max_count = 5;
uint8_t current_mode_num = 0;

uint16_t calc_pwm(uint32_t t)
{
    // t - seconds
    // _/^_/^_/^
    uint16_t n1 = 30; // low
    uint16_t n2 = 7; // max
    uint16_t  n = n1 + n2;
    uint16_t _t = (uint16_t)(t % n);

    if (_t <= n1)
    {
        return 80;
    }
    return 300;
}


uint16_t calc_pwm2(uint32_t t)
{
    // t - seconds
    // _/^\__/^\__/^\_
    uint16_t n1 = 1;
    uint16_t n2 = 3;
    uint16_t n3 = 1;
    uint16_t np = 33;
    uint16_t n = n1 + np + n2 + np + n3;
    uint16_t _t = (uint16_t)(t % n);
    if (_t < n1)
    {
        return 0;
    }
    if (_t < (n1 + np))
    {
        return (_t - n1) * 10;
    }
    if (_t < (n1 + np + n2))
    {
        return 320;
    }
    if (_t < (n1 + np + n2 + np))
    {
        return 320 - ((_t - (n1 + np + n2)) * 10);
    }
    // if _t >= (n1 + np + n2 + np):
    return 0;
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

void update_pwm_pin9(uint16_t new_pwm_value)
{
    if (new_pwm_value != pwm_old)
    {
        OCR1A = new_pwm_value;  // pin 9
        pwm_old = new_pwm_value;
        Serial.print("New pwm\n");
        Serial.print(new_pwm_value);
        Serial.print("\n");
    }
}

void setup()
{
    Serial.begin(9600);
    delay(100);
    // вход выключателя к +, замыкать на gnd
    //  pinMode(BUTTON_PIN, INPUT_PULLUP); // для обычного выключателя или кнопки
    //  pinMode(BUTTON_PIN, INPUT);  // для ttp223
    set_pwm_pin9();

    EEPROM.get(addr_current_mode_num, current_mode_num);
    if (current_mode_num >= modes_max_count)
    {
        current_mode_num = 0;
    }
    Serial.print("Load mode num from eeprom:");
    Serial.print(current_mode_num);
    Serial.print("\n");
    update_pwm_current_mode();
}

void blink(uint8_t n_blinks)
{
    for (uint8_t i = 0; i < n_blinks; i++)
    {
        digitalWrite(13, HIGH);
        delay(80);
        digitalWrite(13, LOW);
        delay(80);
    }
    delay(200);
}

uint16_t get_new_pwm(FAN_MODE m)
{
    switch (m)
    {
        case MODE_MAX_SPEED:
            return 320;
        case MODE_MID_SPEED:
            return 210;
        case MODE_LOW_SPEED:
            return 140;
        case MODE_SMOOTH_MIN_MAX:
            return calc_pwm2(millis() / 1000);
        case MODE_SWITCH_MIN_MAX:
            return calc_pwm(millis() / 1000);
        default:
            return 320;
    }
    return 320;
}

void set_next_mode()
{
    current_mode_num = (current_mode_num + 1) % modes_max_count;
    EEPROM.put(addr_current_mode_num, current_mode_num);
}

void update_pwm_current_mode()
{
    uint16_t pwm_x = get_new_pwm(modes[current_mode_num]);
    update_pwm_pin9(pwm_x);
}

void set_pwm_current_mode(FAN_MODE m)
{
    for (uint16_t i = 0; i < modes_max_count; i++)
    {
        if (modes[i] == m)
        {
            current_mode_num = i;
            break;
        }
    }
    // if unknown mode - no change current
}

void loop()
{
    button.tick();

    if (button.click())
    {
        Serial.print("Button click\n");
        set_next_mode();
        blink(1);
    }

    if (button.holding())
    {
        Serial.print("Button holding\n");
        set_pwm_current_mode(MODE_MAX_SPEED);
        blink(2);
    }

    update_pwm_current_mode();
}
