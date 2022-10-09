#ifndef CONFIG_H
#define CONFIG_H

#include "request.h"

#define CONFIG_STRUCT_SIZE 512
/*
 *  This is better than defining [...]_ELEMENTS_COUNT as it's less error prone.
 */
#define CONFIG_STRUCT_CRITICAL_DATA_SIZE REQUEST_NET_SSID_LEN + 1 +          \
                                             REQUEST_NET_PASS_LEN + 1 +      \
                                             REQUEST_API_USERNAME_LEN + 1 +  \
                                             REQUEST_API_FEED_NAME_LEN + 1 + \
                                             REQUEST_API_KEY_LEN + 1 + 1 // For 1 time setup element

#define CONFIG_STRUCT_LEFT_SPACE (CONFIG_STRUCT_SIZE - (CONFIG_STRUCT_CRITICAL_DATA_SIZE))

typedef struct
{
    bool oneTimeSetupDone;
    char ssid[REQUEST_NET_SSID_LEN + 1];
    char password[REQUEST_NET_PASS_LEN + 1];
    char username[REQUEST_API_USERNAME_LEN + 1];
    char feedName[REQUEST_API_FEED_NAME_LEN + 1];
    char apiKey[REQUEST_API_KEY_LEN + 1];
    /*
     * Data length must be a multiple of page size,
     * this is basically here, not to waste that space.
     */
    char message[CONFIG_STRUCT_LEFT_SPACE];
} config_t;

void configLoad(config_t *config);
void configSave(config_t *config);
bool configApplyDefaults(bool force);
void configHandler();

#endif