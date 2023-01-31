/*
 * File: serial.h
 * Project: Klik
 * -----
 * This source code is released under BSD-3 license.
 * Check LICENSE file for full list of conditions and disclaimer.
 * -----
 * Copyright 2022 - 2023 M.Kusiak (timax)
 */

#ifndef SERIAL_H
#define SERIAL_H

#define SERIAL_MAX_INCOME_LEN 256

void serialUartInit();
char *serialUartGetLastLine();
char *serialUsbGetLastLine();
bool serialUartSendLine(char *line);
void serialUartSetInterruptHandler(void *handlerFunction);

#endif