/*
 * File: picow_tls_client.c
 * Project: Klik
 * -----
 * This source code is released under BSD-3 license.
 * Check LICENSE file for full list of conditions and disclaimer.
 * -----
 * Copyright 2022 - 2022 M.Kusiak (timax)
 */

/**
 * Copyright 2020 (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
 * following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following
 *    disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * This is modified version of picow_tls_client example to be used as library.
 *
 * At the time of writing this (30.09.2022) example is not yet part of the main branch.
 * The example has been taken from one of development branches.
 *
 * As for now, this (converting to library) is done with minimum effort as this code is a bit of a mess,
 * and will probably change till merge.
 *
 * It's ok for what I need it at this moment.
 */

#include <string.h>
#include <time.h>

#include "hardware/structs/rosc.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/altcp_tcp.h"
#include "lwip/altcp_tls.h"
#include "lwip/dns.h"

#include "picow_tls_client.h"

#define RESPONSE_BUF_SIZE 4096

static struct altcp_tls_config *tls_config = NULL;

static char g_responseBuf[RESPONSE_BUF_SIZE];

/* Function to feed mbedtls entropy. May be better to move it to pico-sdk */
int mbedtls_hardware_poll(void *data, unsigned char *output, size_t len, size_t *olen)
{
    /* Code borrowed from pico_lwip_random_byte(), which is static, so we cannot call it directly */
    static uint8_t byte;

    for (int p = 0; p < len; p++)
    {
        for (int i = 0; i < 32; i++)
        {
            // picked a fairly arbitrary polynomial of 0x35u - this doesn't have to be crazily uniform.
            byte = ((byte << 1) | rosc_hw->randombit) ^ (byte & 0x80u ? 0x35u : 0);
            // delay a little because the random bit is a little slow
            busy_wait_at_least_cycles(30);
        }
        output[p] = byte;
    }

    *olen = len;
    return 0;
}

static err_t tls_client_close(void *arg)
{
    TLS_CLIENT_T *state = (TLS_CLIENT_T *)arg;
    err_t err = ERR_OK;

    state->complete = true;
    if (state->pcb != NULL)
    {
        altcp_arg(state->pcb, NULL);
        altcp_poll(state->pcb, NULL, 0);
        altcp_recv(state->pcb, NULL);
        altcp_err(state->pcb, NULL);
        err = altcp_close(state->pcb);
        if (err != ERR_OK)
        {
            // printf("TLS: close failed %d, calling abort\n", err);
            altcp_abort(state->pcb);
            err = ERR_ABRT;
        }
        state->pcb = NULL;
    }
    return err;
}

static err_t tls_client_connected(void *arg, struct altcp_pcb *pcb, err_t err)
{
    TLS_CLIENT_T *state = (TLS_CLIENT_T *)arg;
    if (err != ERR_OK)
    {
        // printf("TLS: connect failed %d\n", err);
        return tls_client_close(state);
    }

    // printf("TLS: connected to server, sending request\n");
    err = altcp_write(state->pcb, state->request, strlen(state->request), TCP_WRITE_FLAG_COPY);
    if (err != ERR_OK)
    {
        // printf("TLS: error writing data, err=%d", err);
        return tls_client_close(state);
    }

    return ERR_OK;
}

static err_t tls_client_poll(void *arg, struct altcp_pcb *pcb)
{
    // printf("TLS: timed out");
    return tls_client_close(arg);
}

static void tls_client_err(void *arg, err_t err)
{
    TLS_CLIENT_T *state = (TLS_CLIENT_T *)arg;
    // printf("TLS: tls_client_err %d\n", err);
    state->pcb = NULL; /* pcb freed by lwip when _err function is called */
}

static err_t tls_client_recv(void *arg, struct altcp_pcb *pcb, struct pbuf *p, err_t err)
{
    TLS_CLIENT_T *state = (TLS_CLIENT_T *)arg;
    if (!p)
    {
        // printf("TLS: connection closed\n");
        return tls_client_close(state);
    }

    if (p->tot_len > 0)
    {
        /* For simplicity this examples creates a buffer on stack the size of the data pending here,
           and copies all the data to it in one go.
           Do be aware that the amount of data can potentially be a bit large (TLS record size can be 16 KB),
           so you may want to use a smaller fixed size buffer and copy the data to it using a loop, if memory is a concern */

        pbuf_copy_partial(p, g_responseBuf, RESPONSE_BUF_SIZE, 0);
        g_responseBuf[RESPONSE_BUF_SIZE - 1] = 0;

        altcp_recved(pcb, p->tot_len);
    }
    pbuf_free(p);

    return ERR_OK;
}

static void tls_client_connect_to_server_ip(const ip_addr_t *ipaddr, TLS_CLIENT_T *state)
{
    err_t err;
    u16_t port = 443;

    // printf("TLS: connecting to server IP %s port %d\n", ipaddr_ntoa(ipaddr), port);
    err = altcp_connect(state->pcb, ipaddr, port, tls_client_connected);
    if (err != ERR_OK)
    {
        fprintf(stderr, "error initiating connect, err=%d\n", err);
        tls_client_close(state);
    }
}

static void tls_client_dns_found(const char *hostname, const ip_addr_t *ipaddr, void *arg)
{
    if (ipaddr)
    {
        // printf("TLS: DNS resolving complete\n");
        tls_client_connect_to_server_ip(ipaddr, (TLS_CLIENT_T *)arg);
    }
    else
    {
        // printf("TLS: error resolving hostname %s\n", hostname);
        tls_client_close(arg);
    }
}

bool tls_client_open(void *arg)
{
    err_t err;
    ip_addr_t server_ip;
    TLS_CLIENT_T *state = (TLS_CLIENT_T *)arg;

    state->pcb = altcp_tls_new(tls_config, IPADDR_TYPE_ANY);
    if (!state->pcb)
    {
        // printf("TLS: failed to create pcb\n");
        return false;
    }

    altcp_arg(state->pcb, state);
    altcp_poll(state->pcb, tls_client_poll, TLS_CLIENT_TIMEOUT_SECS * 2);
    altcp_recv(state->pcb, tls_client_recv);
    altcp_err(state->pcb, tls_client_err);

    /* Set SNI */
    mbedtls_ssl_set_hostname(altcp_tls_context(state->pcb), state->hostname);

    // printf("TLS: resolving %s\n", state->hostname);

    // cyw43_arch_lwip_begin/end should be used around calls into lwIP to ensure correct locking.
    // You can omit them if you are in a callback from lwIP. Note that when using pico_cyw_arch_poll
    // these calls are a no-op and can be omitted, but it is a good practice to use them in
    // case you switch the cyw43_arch type later.
    cyw43_arch_lwip_begin();

    err = dns_gethostbyname(state->hostname, &server_ip, tls_client_dns_found, state);
    if (err == ERR_OK)
    {
        /* host is in DNS cache */
        tls_client_connect_to_server_ip(&server_ip, state);
    }
    else if (err != ERR_INPROGRESS)
    {
        // printf("TLS: error initiating DNS resolving, err=%d\n", err);
        tls_client_close(state->pcb);
    }

    cyw43_arch_lwip_end();

    return err == ERR_OK || err == ERR_INPROGRESS;
}

// Perform initialisation
TLS_CLIENT_T *tls_client_init(void)
{
    TLS_CLIENT_T *state = calloc(1, sizeof(TLS_CLIENT_T));
    if (!state)
    {
        // printf("TLS: failed to allocate state\n");
        return NULL;
    }

    return state;
}

void altcp_tls_config_client_init()
{
    tls_config = altcp_tls_create_config_client(NULL, 0);
}

void altcp_tls_config_client_free()
{
    altcp_tls_free_config(tls_config);
}

void altcp_tls_poll_cyw43()
{
    cyw43_arch_poll();
}

char *altcp_tls_get_response_buffer()
{
    return g_responseBuf;
}

// true if success
bool altcp_tls_setup_cyw43(char *ssid, char *password, uint timeout)
{
    if (cyw43_arch_init())
    {
        // printf("TLS: failed to initialise\n");
        return false;
    }
    cyw43_arch_enable_sta_mode();

    if (cyw43_arch_wifi_connect_timeout_ms(ssid, password, CYW43_AUTH_WPA2_AES_PSK, timeout))
    {
        // printf("TLS: failed to connect\n");
        return false;
    }

    return true;
}