#include <stdio.h>
#include <string.h>
#include <ctype.h>
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

#define RESPONSE_VALUE_STRING "\"value\":\""
#define TAP_BREAK_TIME 500

typedef enum
{
    KLIK_STATE_SETUP,
    KLIK_STATE_CONNECTING,
    KLIK_STATE_CONNECTION_ERROR,
    KLIK_STATE_REQUEST_ERROR,
    KLIK_STATE_WORKING,
    KLIK_STATE_UNDEFINED
} state_t;

typedef enum
{
    KLIK_MODE_OFF,
    KLIK_MODE_ON,
    KLIK_MODE_TAP,
    KLIK_MODE_DOUBLE_TAP
} klik_mode_t;

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
 * @brief Gets the value from request.
 *
 * @param response request response.
 * @return int8_t  value from request,
 *                 if value negative then bad request or overfilled value.
 */
int8_t getValueFromResponse(char *response)
{
    int8_t value = 0;
    char *valueString = strstr(response, RESPONSE_VALUE_STRING);

    if (!valueString)
        return -1;

    valueString += strlen(RESPONSE_VALUE_STRING);

    while (isdigit(valueString[0]))
    {
        value *= 10;
        value += valueString[0] - '0';
        valueString++;
    }

    return value;
}

/**
 * @brief Moves servo to corner positions by value.
 *
 * @param max true if to max allowed angle, false to position 0.
 * @param maxAngle maximum angle of motion.
 */
void moveServoByValue(bool max, uint8_t maxAngle)
{
    float angle = (float)maxAngle;
    if (max)
        servoMoveToAngle(SERVO_PIN, angle);
    else
        servoMoveToAngle(SERVO_PIN, 0);
}

/**
 * @brief Moves servo back and forth.
 *
 * @param count     how many times of back and forth motion.
 * @param maxAngle  maximum angle of motion.
 */
void servoTap(uint8_t count, uint8_t maxAngle)
{
    bool tapState = false;
    uint8_t cycles = (2 * count) + 1;

    for (int i = 0; i < cycles; i++)
    {
        moveServoByValue(tapState, maxAngle);
        tapState = !tapState;
        sleep_ms(TAP_BREAK_TIME);
    }
}

/**
 * @brief main(), lol.
 */
int main()
{
    config_t config;
    char *request, *response;
    int8_t responseValue;

    stdio_init_all();

    /*
     * INITIAL SETUP
     */

    diodeSetState(KLIK_STATE_SETUP);
    configLoad(&config);
    configApplyDefaults(false);
    serialInit();
    serialSetInterruptHandler(configHandler);
    servoSetup(SERVO_PIN);
    buttonSet(BUTTON_PIN);

    /*
     * CONNECT TO WI-FI
     */

    diodeSetState(KLIK_STATE_CONNECTING);

    if (!requestSetup(config.ssid, config.password))
    {
        diodeSetState(KLIK_STATE_CONNECTION_ERROR);
        goto error;
    }

    /*
     * SEND INITIAL REQUEST
     */

    diodeSetState(KLIK_STATE_WORKING);

    request = requestPrepareGET(config.username, config.feedName, config.apiKey);
    response = requestSend(request);
    responseValue = getValueFromResponse(response);

    if (responseValue < 0)
    {
        diodeSetState(KLIK_STATE_REQUEST_ERROR);
        goto error;
    }

    /*
     * LOOP PHRASE
     * At this phrase everything should be working.
     */

    while (1)
    {
        request = requestPrepareGET(config.username, config.feedName, config.apiKey);
        response = requestSend(request);
        responseValue = getValueFromResponse(response);

        /*
         * Overwrite responseValue if button pressed
         */
        if (buttonReadState(BUTTON_PIN) &&
            (responseValue == KLIK_MODE_ON || responseValue == KLIK_MODE_OFF))
        {
            responseValue = !responseValue;
            request = requestPreparePOST(responseValue, config.username, config.feedName, config.apiKey);
            requestSend(request);
        }

        switch (responseValue)
        {
        case KLIK_MODE_OFF:
            moveServoByValue(KLIK_MODE_OFF, config.angleMax);
            break;
        case KLIK_MODE_ON:
            moveServoByValue(KLIK_MODE_ON, config.angleMax);
            break;
        case KLIK_MODE_TAP:
            servoTap(1, config.angleMax);
            request = requestPreparePOST(KLIK_MODE_OFF, config.username, config.feedName, config.apiKey);
            requestSend(request);
            break;
        case KLIK_MODE_DOUBLE_TAP:
            servoTap(2, config.angleMax);
            request = requestPreparePOST(KLIK_MODE_OFF, config.username, config.feedName, config.apiKey);
            requestSend(request);
            break;
        default:
            break;
        }

        sleep_ms(1000);
    }

    /*
     * ERROR
     * End here if anything fails during first 3 phrases.
     * Device will blink LED, with error code.
     */

error:
    while (1)
    {
        tight_loop_contents();
    }

    return 0;
}
