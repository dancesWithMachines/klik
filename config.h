/*
 * File: config.h
 * Project: Klik
 * -----
 * This source code is released under BSD-3 license.
 * Check LICENSE file for full list of conditions and disclaimer.
 * -----
 * Copyright 2022 - 2022 M.Kusiak (timax)
 */

#ifndef CONFIG_H
#define CONFIG_H

#include "request.h"

#define CONFIG_STRUCT_SIZE 512
#define CONFIG_LEN_FIRST_TIME_SETUP 1
#define CONFIG_LEN_MAX_ANGLE 1
/*
 *  This is better than defining [...]_ELEMENTS_COUNT as it's less error prone.
 */
#define CONFIG_STRUCT_CRITICAL_DATA_SIZE REQUEST_NET_SSID_LEN + 1 +          \
                                             REQUEST_NET_PASS_LEN + 1 +      \
                                             REQUEST_API_USERNAME_LEN + 1 +  \
                                             REQUEST_API_FEED_NAME_LEN + 1 + \
                                             REQUEST_API_KEY_LEN + 1 +       \
                                             CONFIG_LEN_FIRST_TIME_SETUP +   \
                                             CONFIG_LEN_MAX_ANGLE

#define CONFIG_STRUCT_LEFT_SPACE (CONFIG_STRUCT_SIZE - (CONFIG_STRUCT_CRITICAL_DATA_SIZE))

typedef struct
{
    bool firstTimeSetup;
    char ssid[REQUEST_NET_SSID_LEN + 1];
    char password[REQUEST_NET_PASS_LEN + 1];
    char username[REQUEST_API_USERNAME_LEN + 1];
    char feedName[REQUEST_API_FEED_NAME_LEN + 1];
    char apiKey[REQUEST_API_KEY_LEN + 1];
    uint8_t angleMax;
    /*
     * Data length must be a multiple of page size,
     * this is basically here, not to waste that space.
     */
    char message[CONFIG_STRUCT_LEFT_SPACE];
} config_t;

void configHandler(char *string);
void configUartInterruptHandler();
void configLoad(config_t *config);
void configSave(config_t *config);
bool configApplyDefaults(bool force);

#endif