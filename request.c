/*
 * File: request.c
 * Project: Klik
 * -----
 * This source code is released under BSD-3 license.
 * Check LICENSE file for full list of conditions and disclaimer.
 * -----
 * Copyright 2022 - 2022 M.Kusiak (timax)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"

#include "request.h"
#include "config.h"

#include "libs/picow_tls_client/picow_tls_client.h"

#define REQUEST_SETUP_TIMEOUT 10000
#define REQUEST_HOSTNAME "io.adafruit.com"

#define REQUEST_PREPARE_TYPE_STRING_LEN 4
#define REQUEST_PREPARE_VALUE_STRING_LEN 16
#define REQUEST_PREPARE_CONNECTION_SIZE_STRING_LEN 21

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
 * @param value         value to be set.
 * @return char*        Request string.
 */
char *prepareRequest(requestType_t type, char *apiUsername, char *apiFeedName, char *apiKey, int8_t value)
{
    static char typeString[REQUEST_PREPARE_TYPE_STRING_LEN + 1];
    static char valueString[REQUEST_PREPARE_VALUE_STRING_LEN + 1];
    static char connectionLengthString[REQUEST_PREPARE_CONNECTION_SIZE_STRING_LEN + 1];

    memset(typeString, 0, sizeof typeString);
    memset(valueString, 0, sizeof valueString);
    memset(connectionLengthString, 0, sizeof connectionLengthString);

    switch (type)
    {
    case REQUEST_GET:
        strncpy(typeString, "GET", 3);
        break;
    case REQUEST_POST:
        strncpy(typeString, "POST", 4);
        snprintf(valueString, REQUEST_PREPARE_VALUE_STRING_LEN, "{\"value\":%d}\r\n", value);
        snprintf(connectionLengthString, REQUEST_PREPARE_CONNECTION_SIZE_STRING_LEN,
                 "Content-Length: %d\r\n", strlen(valueString));
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
char *requestPreparePOST(int8_t value, char *apiUsername, char *apiFeedName, char *apiKey)
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