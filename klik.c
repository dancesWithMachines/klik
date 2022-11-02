#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"

#include "button.h"
#include "serial.h"
#include "led.h"
#include "servo.h"
#include "request.h"
#include "config.h"

#define BUTTON_PIN 26

#define LED_BLUE 20
#define LED_GREEN 19
#define LED_RED 18

#define SERVO_PIN 21

#define RESPONSE_VALUE_1 "\"value\":\"1\""
#define RESPONSE_VALUE_0 "\"value\":\"0\""

typedef enum
{
    KLIK_STATE_SETUP,
    KLIK_STATE_CONNECTING,
    KLIK_STATE_CONNECTION_ERROR,
    KLIK_STATE_REQUEST_ERROR,
    KLIK_STATE_WORKING,
    KLIK_STATE_UNDEFINED
} state_t;

/**
 * @brief Blinks led diode according to state.
 *
 * @param state current device state.
 */
void diodeSetState(state_t state)
{
    static bool firstTimeSetupDone;
    static ledDiode_t led;
    static struct repeating_timer timer;

    if (!firstTimeSetupDone)
    {
        ledDiodeSetup(&led, LED_BLUE, LED_GREEN, LED_RED);
        firstTimeSetupDone = true;
    }
    else
    {
        cancel_repeating_timer(&timer);
        ledDiodeDim(&led);
    }

    switch (state)
    {
    case KLIK_STATE_SETUP:
        ledCycle(&led, &timer, 200, COLOR_GREEN, COLOR_BLUE);
        break;
    case KLIK_STATE_CONNECTING:
        ledCycle(&led, &timer, 200, COLOR_BLUE, COLOR_RED);
        break;
    case KLIK_STATE_CONNECTION_ERROR:
        ledBlink(&led, &timer, COLOR_RED, 1000);
        break;
    case KLIK_STATE_REQUEST_ERROR:
        ledBlink(&led, &timer, COLOR_BLUE, 1000);
        break;
    case KLIK_STATE_WORKING:
        ledBlink(&led, &timer, COLOR_GREEN, 5000);
        break;
    case KLIK_STATE_UNDEFINED:
        break;
    }
}

/**
 * @brief Checks if return value is 0 or 1 and returns it.
 *
 * @param response response.
 * @return int8_t  0 or 1 as value, -1 if bad request.
 */
int8_t getValueByResponse(char *response)
{
    if (strstr(response, RESPONSE_VALUE_0) != NULL)
        return false;
    else if (strstr(response, RESPONSE_VALUE_1) != NULL)
        return true;
    else
        return -1;
}

/**
 * @brief Moves servo to corner positions by value.
 *
 * @param moveUp true if up, false otherwise.
 */
void moveServoByValue(bool moveUp)
{
    if (moveUp)
        servoMoveUp(SERVO_PIN);
    else
        servoMoveDown(SERVO_PIN);
}

/**
 * @brief main(), lol.
 */
int main()
{
    config_t config;
    char *request, *response;
    bool value;
    requestType_t lastRequestType;
    int8_t responseStatus;

    stdio_init_all();
    diodeSetState(KLIK_STATE_SETUP);
    configLoad(&config);
    configApplyDefaults(false);
    serialInit();
    serialSetInterruptHandler(configHandler);
    servoInit(SERVO_PIN);
    buttonSet(BUTTON_PIN);
    diodeSetState(KLIK_STATE_CONNECTING);

    if (!requestSetup(config.ssid, config.password))
    {
        diodeSetState(KLIK_STATE_CONNECTION_ERROR);
        goto error;
    }

    diodeSetState(KLIK_STATE_WORKING);

    request = requestPrepareGET(config.username, config.feedName, config.apiKey);
    response = requestSend(request);
    responseStatus = getValueByResponse(response);

    if (responseStatus < 0)
    {
        diodeSetState(KLIK_STATE_REQUEST_ERROR);
        goto error;
    }

    value = (bool)responseStatus;
    lastRequestType = REQUEST_GET;

    while (1)
    {
        if (buttonReadState(BUTTON_PIN))
        {
            value = !value;
            request = requestPreparePOST(value, config.username, config.feedName, config.apiKey);
            lastRequestType = REQUEST_POST;
        }
        else if (lastRequestType == REQUEST_POST)
        {
            request = requestPrepareGET(config.username, config.feedName, config.apiKey);
            lastRequestType = REQUEST_GET;
        }

        response = requestSend(request);
        responseStatus = getValueByResponse(response);

        if (responseStatus >= 0 && lastRequestType != REQUEST_POST)
            value = (bool)responseStatus;

        moveServoByValue(value);
        sleep_ms(1000);
    }

error:
    while (1)
    {
        tight_loop_contents();
    }

    return 0;
}
