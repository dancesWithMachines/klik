/*
 * File: servo.h
 * Project: Klik
 * -----
 * This source code is released under BSD-3 license.
 * Check LICENSE file for full list of conditions and disclaimer.
 * -----
 * Copyright 2022 - 2022 M.Kusiak (timax)
 */

#ifndef SERVO_H
#define SERVO_H

#define SERVO_MAX_ANGLE 180

void servoSetup(uint8_t servoPin);
void servoMoveToAngle(uint8_t servoPin, float degree);

#endif