/*
 * File: button.h
 * Project: Klik
 * -----
 * This source code is released under BSD-3 license.
 * Check LICENSE file for full list of conditions and disclaimer.
 * -----
 * Copyright 2022 - 2022 M.Kusiak (timax)
 */

#ifndef BUTTON_H
#define BUTTON_H

void buttonSet(uint8_t buttonPin);
bool buttonReadState(uint8_t buttonPin);

#endif