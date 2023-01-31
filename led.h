/*
 * File: led.h
 * Project: Klik
 * -----
 * This source code is released under BSD-3 license.
 * Check LICENSE file for full list of conditions and disclaimer.
 * -----
 * Copyright 2022 - 2023 M.Kusiak (timax)
 */

#ifndef LED_H
#define LED_H

#define LED_BLINK_TIME 200

typedef struct
{
    uint8_t bluePin;
    uint8_t greenPin;
    uint8_t redPin;
} ledDiode_t;

typedef enum
{
    COLOR_NONE,
    COLOR_BLUE,
    COLOR_GREEN,
    COLOR_RED
} color_t;

void ledDiodeInit(ledDiode_t *diode);
void ledDiodeSetup(ledDiode_t *diode, uint8_t bluePin, uint8_t greenPin, uint8_t redPin);
void ledDiodeDim(ledDiode_t *diode);
void ledCycle(ledDiode_t *diode, struct repeating_timer *timer, uint delay, color_t color1, color_t color2);
void ledBlink(ledDiode_t *diode, struct repeating_timer *timer, color_t color, uint delay);
void ledTest();

#endif