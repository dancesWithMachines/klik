#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"

#include "request.h"
#include "config.h"

#include "libs/picow_tls_client/picow_tls_client.h"

#define REQUEST_SETUP_TIMEOUT 10000
#define REQUEST_HOSTNAME "io.adafruit.com"

/*
 * I don't like defining this globally,
 * but I'd rather separate "networking" from the rest of the code.
 */
static TLS_CLIENT_T *g_client;

/*
 * I am aware that this approach is questionable,
 * but I think given the client that tls client is in,
 * theres no point on making it "easy to adapt" or universal.
 *
 * The requests will be send one after one anyway.
 * It does what I need it to do.
 */
static char g_request[REQUEST_API_FORM_MAX_LEN + 1];

/**
 * @brief Prepares http request string for Adafruit IO HTTP API.
 *
 * @param type          type of the request, (REQUESTS_)GET or POST.
 * @param apiUsername   owner's (account) username.
 * @param apiFeedName   feed name.
 * @param apiKey        api key.
 * @param value         value to be set. For now only single digit value is supported.
 *                      We'll flip between 0 and 1 anyway.
 * @return char*        Request string.
 */
char *prepareRequest(requestType_t type, char *apiUsername, char *apiFeedName, char *apiKey, uint8_t value)
{
    char typeString[5];
    char valueString[16];
    char connectionLengthString[21]; // Value can be single digit only, added to TODO

    switch (type)
    {
    case REQUEST_GET:
        strncpy(typeString, "GET", 3);
        break;
    case REQUEST_POST:
        strncpy(typeString, "POST", 5);
        strncpy(connectionLengthString, "Content-Length: 11\r\n", 20); // Value can be single digit only
        snprintf(valueString, 14, "{\"value\":%d}\r\n", value);
        break;
    }

    snprintf(g_request, REQUEST_API_FORM_MAX_LEN + 1,
             "%s /api/v2/%s/feeds/%s/data HTTP/1.1\r\n"
             "X-AIO-Key: %s\r\n"
             "Content-Type: application/json\r\n"
             "Accept: */*\r\n"
             "Host: " REQUEST_HOSTNAME "\r\n"
             "Connection: close\r\n" // Otherwise timeout will happen
             "%s"
             "\r\n"
             "%s",
             typeString, apiUsername, apiFeedName,
             apiKey,
             connectionLengthString,
             valueString);

    return g_request;
}

/**
 * @brief Setups everything that is needed to send http requests.
 *        This includes cyw43 (aka onboard wifi) and tls client stuff.
 *
 * @param ssid      network's SSID.
 * @param password  network password.
 * @return true     if everything has been setup correctly.
 * @return false    if setup went wrong.
 */
bool requestSetup(char *ssid, char *password)
{
    if (!altcp_tls_setup_cyw43(ssid, password, REQUEST_SETUP_TIMEOUT))
        return false;

    altcp_tls_config_client_init();

    if (!g_client)
        g_client = tls_client_init();
    if (!g_client)
        return false;

    return true;
}

/**
 * @brief Prepares http Get request for Adafruit IO HTTP API.
 *
 * @param apiUsername owner's (account) username.
 * @param apiFeedName feed name.
 * @param apiKey      api key.
 * @return char*      Http GET request string.
 */
char *requestPrepareGET(char *apiUsername, char *apiFeedName, char *apiKey)
{
    return prepareRequest(REQUEST_GET, apiUsername, apiFeedName, apiKey, 0);
}

/**
 * @brief Prepares http POST request for Adafruit IO HTTP API.
 *
 * @param value       value to be set. For now only single digit value is supported.
 *                    We'll flip between 0 and 1 anyway.
 * @param apiUsername owner's (account) username.
 * @param apiFeedName feed name.
 * @param apiKey      api key.
 * @return char*      Http GET request string.
 */
char *requestPreparePOST(uint8_t value, char *apiUsername, char *apiFeedName, char *apiKey)
{
    return prepareRequest(REQUEST_POST, apiUsername, apiFeedName, apiKey, value);
}

/**
 * @brief Sends http request to Adafruit IO HTTP API.
 *
 * @param request request.
 * @return char*  http response.
 */
char *requestSend(char *request)
{
    static TLS_CLIENT_T client;

    client.hostname = REQUEST_HOSTNAME;
    client.request = request;

    if (!tls_client_open(&client))
        return NULL;
    while (!client.complete)
    {
        altcp_tls_poll_cyw43();
        sleep_ms(1);
    }

    client.complete = false;

    return altcp_tls_get_response_buffer();
}

/**
 * @brief Destroy requests. Frees memory, etc.
 */
void requestDestroy()
{
    free(g_client);
    altcp_tls_config_client_free();
}