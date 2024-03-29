/*
 * File: request.h
 * Project: Klik
 * -----
 * This source code is released under BSD-3 license.
 * Check LICENSE file for full list of conditions and disclaimer.
 * -----
 * Copyright 2022 - 2023 M.Kusiak (timax)
 */

#ifndef REQUEST_H
#define REQUEST_H

#define REQUEST_NET_SSID_LEN 32
#define REQUEST_NET_PASS_LEN 64

#define REQUEST_API_USERNAME_LEN 30
#define REQUEST_API_FEED_NAME_LEN 128
#define REQUEST_API_KEY_LEN 32
// The form will be less than that but let's leave some safe space
#define REQUEST_API_FORM_ALONE_LEN 200
#define REQUEST_API_FORM_MAX_LEN REQUEST_API_USERNAME_LEN +      \
                                     REQUEST_API_FEED_NAME_LEN + \
                                     REQUEST_API_KEY_LEN +       \
                                     REQUEST_API_FORM_ALONE_LEN

typedef enum
{
    REQUEST_GET,
    REQUEST_POST
} requestType_t;

bool requestSetup(char *ssid, char *password);
char *requestPrepareGET(char *apiUsername, char *apiFeedName, char *apiKey);
char *requestPreparePOST(int8_t value, char *apiUsername, char *apiFeedName, char *apiKey);
char *requestSend(char *request);
void requestDestroy();

#endif