/*
 * File: led.c
 * Project: Klik
 * -----
 * This source code is released under BSD-3 license.
 * Check LICENSE file for full list of conditions and disclaimer.
 * -----
 * Copyright 2022 - 2023 M.Kusiak (timax)
 */

#include <stdio.h>
#include "pico/stdlib.h"

#include "led.h"

/**
 * @brief Converts miliseconds to microseconds.
 */
#define MS_TO_US(m) ((m) * (1000))

/**
 * @brief A structure for blink pattern.
 */
typedef struct
{
    uint8_t pin;
    uint delay;
    uint length;
} blinkPattern_t;

/**
 * @brief Get diode's pin corresponding to color.
 *
 * @param diode     diode.
 * @param color     color.
 * @return uint8_t  pin number.
 */
uint8_t getPinByColor(ledDiode_t *diode, color_t color)
{
    switch (color)
    {
    case COLOR_BLUE:
        return diode->bluePin;
    case COLOR_GREEN:
        return diode->greenPin;
    case COLOR_RED:
        return diode->redPin;
    default:
        return 0;
    }
}

/**
 * @brief Cycle power between two led pins. Used as callback.
 *
 * @param timer timer.
 * @return true
 */
bool cycle(struct repeating_timer *timer)
{
    uint8_t *pins = (char *)timer->user_data;

    gpio_put(pins[0], !gpio_get_out_level(pins[0]));
    gpio_put(pins[1], !gpio_get_out_level(pins[1]));

    return true;
}

/**
 * @brief Blink diode shortly.
 *
 * @param timer timer.
 * @return true
 */
bool blink(struct repeating_timer *timer)
{
    blinkPattern_t *pattern = (blinkPattern_t *)timer->user_data;

    if (gpio_get_out_level(pattern->pin))
    {
        gpio_put(pattern->pin, 0);
        timer->delay_us = MS_TO_US(pattern->delay);
    }
    else
    {
        gpio_put(pattern->pin, 1);
        timer->delay_us = MS_TO_US(pattern->length);
    }

    return true;
}

/**
 * @brief Initiates and sets direction for diode pins.
 *
 * @param diode diode.
 */
void ledDiodeInit(ledDiode_t *diode)
{
    gpio_init(diode->bluePin);
    gpio_init(diode->greenPin);
    gpio_init(diode->redPin);
    gpio_set_dir(diode->bluePin, GPIO_OUT);
    gpio_set_dir(diode->greenPin, GPIO_OUT);
    gpio_set_dir(diode->redPin, GPIO_OUT);
}

/**
 * @brief Quickly setups diode. Sets the pins, initiates, and sets direction.
 *
 * @param diode     diode.
 * @param bluePin   blue pin.
 * @param greenPin  green pin.
 * @param redPin    red pin.
 */
void ledDiodeSetup(ledDiode_t *diode, uint8_t bluePin, uint8_t greenPin, uint8_t redPin)
{
    diode->bluePin = bluePin;
    diode->greenPin = greenPin;
    diode->redPin = redPin;

    ledDiodeInit(diode);
}

/**
 * @brief Switches led diode off.
 *
 * @param diode diode.
 */
void ledDiodeDim(ledDiode_t *diode)
{
    gpio_put(diode->bluePin, 0);
    gpio_put(diode->greenPin, 0);
    gpio_put(diode->redPin, 0);
}

/**
 * @brief Cycles diode colors by specified time.
 * Function is non blocking, colors cycle independent from code.
 *
 * @param diode     diode.
 * @param timer     timer.
 * @param delay     delay between color changes.
 * @param color1    first color.
 * @param color2    second color.
 */
void ledCycle(ledDiode_t *diode, struct repeating_timer *timer, uint delay, color_t color1, color_t color2)
{
    static uint8_t pins[2];

    pins[0] = getPinByColor(diode, color1);
    pins[1] = getPinByColor(diode, color2);

    gpio_put(pins[0], 1);
    gpio_put(pins[1], 0);
    add_repeating_timer_ms(delay, cycle, &pins, timer);
}

/**
 * @brief Blinks led every given time for LED_BLINK_TIME
 *
 * @param diode diode.
 * @param timer timer.
 * @param color color.
 * @param delay delay between blinks.
 */
void ledBlink(ledDiode_t *diode, struct repeating_timer *timer, color_t color, uint delay)
{
    static blinkPattern_t pattern;

    pattern.delay = delay;
    pattern.length = LED_BLINK_TIME;
    pattern.pin = getPinByColor(diode, color);

    add_repeating_timer_ms(500, blink, &pattern, timer);
}
