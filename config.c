/*
 * File: config.c
 * Project: Klik
 * -----
 * This source code is released under BSD-3 license.
 * Check LICENSE file for full list of conditions and disclaimer.
 * -----
 * Copyright 2022 - 2023 M.Kusiak (timax)
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "hardware/sync.h"

#include "dictionary.h"
#include "serial.h"
#include "config.h"
#include "servo.h"

#define CONFIG_SEPARATOR 1
#define CONFIG_MODE_LEN 3
#define CONFIG_SETTING_BEGIN (CONFIG_MODE_LEN + CONFIG_SEPARATOR)
#define CONFIG_SETTING_LEN 4
#define CONFIG_VALUE_BEGIN (CONFIG_SETTING_BEGIN + CONFIG_SETTING_LEN + CONFIG_SEPARATOR)
#define CONFIG_VALUE_LEN (SERIAL_MAX_INCOME_LEN - CONFIG_VALUE_BEGIN)

#define CONFIG_MESSAGE_MODE_UNSUPPORTED "MODE UNSUPPORTED"
#define CONFIG_MESSAGE_SETTING_UNSUPPORTED "SETTING UNSUPPORTED"
#define CONFIG_MESSAGE_SUCCESS "SUCCESS"

#define CONFIG_DEFAULT_SSID "NETWORK SSID"
#define CONFIG_DEFAULT_PASSWORD "NETWORK PASSWORD"
#define CONFIG_DEFAULT_USERNAME "API USERNAME"
#define CONFIG_DEFAULT_FEEDNAME "API FEED NAME"
#define CONFIG_DEFAULT_API_KEY "API KEY"
#define CONFIG_DEFAULT_MESSAGE "Let's put weird and weird together,\n" \
                               "and make it even weirder!\n"           \
                               "Weird, weird space is\n"               \
                               "super-weird!"
/**
 * @brief This is 2MB (Pico flash memory size) - 4KB (Block size) in bytes.
 * In other words, this is the offset to last writable memory sector.
 */
#define MEMORY_OFFSET 2093056

/**
 * @brief This is enum for storing settings (configuration entries)
 */
typedef enum
{
    SETTING_SSID,
    SETTING_PASSWORD,
    SETTING_USERNAME,
    SETTING_FEED_NAME,
    SETTING_API_KEY,
    SETTING_ANGLE_MAX,
    SETTING_MESSAGE,
    SETTING_ALL,
    SETTING_UNDEFINED
} setting_t;

/**
 * @brief This is dictionary that binds setting (configuration entry) with setting string.
 */
dictionary_t settingsDictionary[] = {
    {SETTING_SSID, "SSID"},
    {SETTING_PASSWORD, "PASS"},
    {SETTING_USERNAME, "USRN"},
    {SETTING_FEED_NAME, "FNME"},
    {SETTING_API_KEY, "APIK"},
    {SETTING_ANGLE_MAX, "ANGL"},
    {SETTING_MESSAGE, "MESS"},
    {SETTING_ALL, "CONF"},
    {SETTING_UNDEFINED, NULL}};

/**
 * @brief  Configuration modes. Supported modes are GET and SET.
 */
typedef enum
{
    CONFIG_MODE_GET,
    CONFIG_MODE_SET,
    CONFIG_MODE_RESET,
    CONFIG_MODE_UNSUPPORTED
} config_mode_t;

/**
 * @brief Mode dictionary. It binds mode with string.
 */
dictionary_t modeDictionary[] = {
    {CONFIG_MODE_GET, "GET"},
    {CONFIG_MODE_SET, "SET"},
    {CONFIG_MODE_RESET, "RST"},
    {CONFIG_MODE_UNSUPPORTED, NULL}};

/**
 * @brief Gets configuration mode based on configuration string.
 *
 * @param string            configuration string.
 * @return config_mode_t    configuration mode (CONFIG_MODE_[...]).
 */
config_mode_t getMode(char *string)
{
    static char mode[CONFIG_MODE_LEN + 1];
    memset(mode, 0, CONFIG_MODE_LEN + 1);
    strncpy(mode, string, CONFIG_MODE_LEN);

    return dictionaryGetEntry(modeDictionary, mode);
}

/**
 * @brief Gets setting (configuration entry) based on configuration string.
 *
 * @param string            configuration string.
 * @return setting_t    setting (SETTING_[...]).
 */
setting_t getSetting(char *string)
{
    static char setting[CONFIG_SETTING_LEN + 1];
    strncpy(setting, &string[CONFIG_SETTING_BEGIN], CONFIG_SETTING_LEN);

    return dictionaryGetEntry(settingsDictionary, setting);
}

/**
 * @brief Gets setting value from configuration string.
 *
 * @param string configuration string.
 * @return char* setting value.
 */
char *getValue(char *string)
{
    static char value[CONFIG_VALUE_LEN + 1];
    int settingLength = strlen(string);

    memset(value, 0, sizeof value);
    strncpy(value, &string[CONFIG_VALUE_BEGIN], settingLength - CONFIG_VALUE_BEGIN + 1); // or even + 2

    return value;
}

/**
 * @brief Handles "Get" configuration mode.
 *        "Get" mode is used to print current settings on serial.
 * @param string configuration setting.
 */
void modeGetHandler(char *string)
{
    config_t config;
    configLoad(&config);

    switch (getSetting(string))
    {
    case SETTING_SSID:
        printf("%s\n", config.ssid);
        break;
    case SETTING_PASSWORD:
        printf("%s\n", config.password);
        break;
    case SETTING_USERNAME:
        printf("%s\n", config.username);
        break;
    case SETTING_FEED_NAME:
        printf("%s\n", config.feedName);
        break;
    case SETTING_API_KEY:
        printf("%s\n", config.apiKey);
        break;
    case SETTING_ANGLE_MAX:
        printf("%d\n", config.angleMax);
        break;
    case SETTING_MESSAGE:
        printf("%s\n", config.message);
        break;
    case SETTING_ALL:
        printf("SSID: %s\n"
               "PASSWORD: %s\n"
               "USERNAME: %s\n"
               "FEED NAME: %s\n"
               "API_KEY: %s\n"
               "MAX ANGLE: %d\n"
               "MESSAGE:%s\n",
               config.ssid,
               config.password,
               config.username,
               config.feedName,
               config.apiKey,
               config.angleMax,
               config.message);
        break;
    case SETTING_UNDEFINED:
        printf("%s\n", CONFIG_MESSAGE_SETTING_UNSUPPORTED);
        break;
    default:
        printf("%s\n", CONFIG_MESSAGE_SETTING_UNSUPPORTED);
        break;
    }
}

/**
 * @brief Handles "Set" configuration mode.
 *        "Set" mode is used to overwrite current settings.
 *        Function sets configuration setting based on string, an (re)writes configuration to onboard flash.
 * @param string configuration string.
 */
void modeSetHandler(char *string)
{
    char *value = getValue(string);
    static uint8_t valueLength;
    static config_t config;

    valueLength = strlen(value);
    configLoad(&config);
    switch (getSetting(string))
    {
    case SETTING_SSID:
        memset(config.ssid, 0, sizeof config.ssid);
        strncpy(config.ssid, value, REQUEST_NET_SSID_LEN);
        break;
    case SETTING_PASSWORD:
        memset(config.password, 0, sizeof config.password);
        strncpy(config.password, value, REQUEST_NET_PASS_LEN);
        break;
    case SETTING_USERNAME:
        memset(config.username, 0, sizeof config.username);
        strncpy(config.username, value, REQUEST_API_USERNAME_LEN);
        break;
    case SETTING_FEED_NAME:
        memset(config.feedName, 0, sizeof config.feedName);
        strncpy(config.feedName, value, REQUEST_API_FEED_NAME_LEN);
        break;
    case SETTING_API_KEY:
        memset(config.apiKey, 0, sizeof config.apiKey);
        strncpy(config.apiKey, value, REQUEST_API_KEY_LEN);
        break;
    case SETTING_ANGLE_MAX:
        config.angleMax = atoi(value);
        if (config.angleMax > SERVO_MAX_ANGLE)
            config.angleMax = SERVO_MAX_ANGLE;
        break;
    case SETTING_MESSAGE:
        memset(config.message, 0, sizeof config.message);
        strncpy(config.message, value, CONFIG_STRUCT_LEFT_SPACE - 1); // This one is different
        break;
    case SETTING_ALL: // Yes, I could skip the contents
        printf("%s\n", CONFIG_MESSAGE_SETTING_UNSUPPORTED);
        return;
    case SETTING_UNDEFINED:
        printf("%s\n", CONFIG_MESSAGE_SETTING_UNSUPPORTED);
        return;
    }

    configSave(&config);
    printf("%s\n", CONFIG_MESSAGE_SUCCESS);
}

/**
 * @brief Handles device configuration standalone.
 *
 * @param string configuration string.
 */
void configHandler(char *string)
{
    switch (getMode(string))
    {
    case CONFIG_MODE_GET:
        modeGetHandler(string);
        break;
    case CONFIG_MODE_SET:
        modeSetHandler(string);
        break;
    case CONFIG_MODE_RESET:
        configApplyDefaults(true);
        break;
    case CONFIG_MODE_UNSUPPORTED:
        printf("%s\n", CONFIG_MESSAGE_MODE_UNSUPPORTED);
        break;
    }
}

/**
 * @brief Handles device configuration over UART (ONLY)!
 *        The function was designed to be run by uart interrupts.
 */
void configUartInterruptHandler()
{
    char *string = serialUartGetLastLine();
    if (!string)
        return;
    configHandler(string);
}

/**
 * @brief Loads configuration from flash.
 *
 * @param config configuration to write to.
 */
void configLoad(config_t *config)
{
    const uint8_t *flash_target_contents = (const uint8_t *)(XIP_BASE + MEMORY_OFFSET);
    memcpy(config, flash_target_contents, CONFIG_STRUCT_SIZE);
}

/**
 * @brief Saves configuration to flash memory.
 * WARNING! This overwrites previous configuration.
 *
 * @param config current configuration.
 */
void configSave(config_t *config)
{
    uint8_t *configBytes = (uint8_t *)config;
    int confSize = CONFIG_STRUCT_SIZE;
    uint32_t interrupts = save_and_disable_interrupts();

    flash_range_erase(MEMORY_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(MEMORY_OFFSET, configBytes, CONFIG_STRUCT_SIZE);
    restore_interrupts(interrupts);
}

/**
 * @brief Checks and saves default configuration to flash if firstTimeSetup is not set.
 *
 * @param force  forces restoring configuration to defaults.
 * @return true  defaults have been restored.
 * @return false skipped saving defaults.
 */
bool configApplyDefaults(bool force)
{
    config_t config;
    configLoad(&config);

    if (!config.firstTimeSetup && !force)
        return false;

    memset(config.ssid, 0, sizeof config.ssid);
    memset(config.password, 0, sizeof config.password);
    memset(config.username, 0, sizeof config.username);
    memset(config.feedName, 0, sizeof config.feedName);
    memset(config.apiKey, 0, sizeof config.apiKey);
    memset(config.message, 0, sizeof config.message);

    strncpy(config.ssid, CONFIG_DEFAULT_SSID, sizeof CONFIG_DEFAULT_SSID);
    strncpy(config.password, CONFIG_DEFAULT_PASSWORD, sizeof CONFIG_DEFAULT_PASSWORD);
    strncpy(config.username, CONFIG_DEFAULT_USERNAME, sizeof CONFIG_DEFAULT_USERNAME);
    strncpy(config.feedName, CONFIG_DEFAULT_FEEDNAME, sizeof CONFIG_DEFAULT_FEEDNAME);
    strncpy(config.apiKey, CONFIG_DEFAULT_API_KEY, sizeof CONFIG_DEFAULT_API_KEY);
    strncpy(config.message, CONFIG_DEFAULT_MESSAGE, sizeof CONFIG_DEFAULT_MESSAGE);

    config.angleMax = SERVO_MAX_ANGLE;
    config.firstTimeSetup = 0;

    configSave(&config);

    if (force)
        printf("%s\n", CONFIG_MESSAGE_SUCCESS);

    return true;
}
