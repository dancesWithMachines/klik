#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

/**
 * @brief Configure a button pin.
 *
 * @param buttonPin button pin.
 */
void buttonSet(uint8_t buttonPin)
{
    gpio_init(buttonPin);
    gpio_set_dir(buttonPin, GPIO_IN);
    gpio_pull_down(buttonPin);
}

/**
 * @brief check if button is pressed.
 *
 * @param buttonPin button pin.
 * @return true - when button pressed, else false.
 */
bool buttonReadState(uint8_t buttonPin)
{
    return gpio_get(buttonPin);
}