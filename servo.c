/*
 * File: servo.c
 * Project: Klik
 * -----
 * This source code is released under BSD-3 license.
 * Check LICENSE file for full list of conditions and disclaimer.
 * -----
 * Copyright 2022 - 2022 M.Kusiak (timax)
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#include "servo.h"

#define SERVO_DIVIDER 64
#define SERVO_WRAP 39062
#define SERVO_CYCLE_LENGTH 20000
#define SERVO_MAXIMUM_LENGTH 2000
#define SERVO_MINIMAL_LENGTH 400

/**
 * @brief Initialise servo.
 *
 * @param servoPin  servo pin.
 */
void servoSetup(uint8_t servoPin)
{
    gpio_set_function(servoPin, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(servoPin);

    pwm_set_wrap(slice, SERVO_WRAP);
    pwm_set_clkdiv(slice, SERVO_DIVIDER);
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
    float multiplier = 1;

    if (degree < SERVO_MAX_ANGLE)
        multiplier = (degree / SERVO_MAX_ANGLE) - ((int)degree / SERVO_MAX_ANGLE);

    float millis = SERVO_MAXIMUM_LENGTH * multiplier + SERVO_MINIMAL_LENGTH;

    pwm_set_gpio_level(servoPin, millis / SERVO_CYCLE_LENGTH * SERVO_WRAP);
}