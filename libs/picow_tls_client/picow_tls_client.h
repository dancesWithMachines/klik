/*
 * File: picow_tls_client.h
 * Project: Klik
 * -----
 * This source code is released under BSD-3 license.
 * Check LICENSE file for full list of conditions and disclaimer.
 * -----
 * Copyright 2022 - 2023 M.Kusiak (timax)
 */

/*
 * This file I made myself.
 */

#ifndef PICOW_TLS_CLIENT_H
#define PICOW_TLS_CLIENT_H

#define TLS_CLIENT_TIMEOUT_SECS 30

typedef struct TLS_CLIENT_T_
{
    struct altcp_pcb *pcb;
    bool complete;
    char *request;
    char *hostname;
} TLS_CLIENT_T;

bool tls_client_open(void *arg);
TLS_CLIENT_T *tls_client_init(void);
void altcp_tls_config_client_init();
void altcp_tls_config_client_free();
void altcp_tls_poll_cyw43();
char *altcp_tls_get_response_buffer();
bool altcp_tls_setup_cyw43(char *ssid, char *password, uint timeout);

#endif