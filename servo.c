#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#include "servo.h"

#define DIVIDER 64
#define WRAP 39062
#define SERVO_CYCLE_LENGTH 20000
#define SERVO_MAXIMUM_LENGTH 2000
#define SERVO_MINIMAL_LENGTH 400

/**
 * @brief Initialise servo.
 *
 * @param servoPin  servo pin.
 */
void servoInit(uint8_t servoPin)
{
    gpio_set_function(servoPin, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(servoPin);

    pwm_set_wrap(slice, WRAP);
    pwm_set_clkdiv(slice, DIVIDER);
    pwm_set_enabled(slice, true);
}

/**
 * @brief Move servo to angle (0-180).
 *
 * @param servoPin  servo pin.
 * @param degree    angle to move to.
 */
void servoMoveToAngle(uint8_t servoPin, float degree)
{
    float multiplier = (degree / 180) - ((int)degree / 180);
    float millis = SERVO_MAXIMUM_LENGTH * multiplier + SERVO_MINIMAL_LENGTH;

    pwm_set_gpio_level(servoPin, millis / SERVO_CYCLE_LENGTH * WRAP);
}

/**
 * @brief Move servo to up position.
 *
 * @param servoPin  servo pin.
 */
void servoMoveUp(uint8_t servoPin)
{
    servoMoveToAngle(servoPin, 0);
}

/**
 * @brief move servo to down position
 *
 * @param servoPin  servo pin.
 */
void servoMoveDown(uint8_t servoPin)
{
    servoMoveToAngle(servoPin, 179);
}
