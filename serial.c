#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "serial.h"

#include "hardware/uart.h"
#include "hardware/irq.h"

#define SERIAL_UART_ID uart0
#define SERIAL_BAUD_RATE 115200
#define SERIAL_DATA_BITS 8
#define SERIAL_STOP_BITS 1
#define SERIAL_PARITY 0

/**
 * @brief Initiates and setups serial communication.
 */
void serialInit()
{
    uart_init(SERIAL_UART_ID, SERIAL_BAUD_RATE);
    uart_set_format(SERIAL_UART_ID, SERIAL_DATA_BITS, SERIAL_STOP_BITS, SERIAL_PARITY);
    uart_set_fifo_enabled(SERIAL_UART_ID, true);
    uart_set_hw_flow(SERIAL_UART_ID, false, false);
}

/**
 * @brief Gets last full line from serial.
 *
 * @return char* Last full line from serial, or 0 when empty.
 */
char *serialGetLastLine()
{
    static char income[SERIAL_MAX_INCOME_LEN];
    static uint16_t incomeIndex;
    char symbol;

    if (!income[0])
        incomeIndex = 0;

    if (!incomeIndex)
        memset(income, 0, sizeof income);

    while (uart_is_readable(SERIAL_UART_ID))
    {
        symbol = uart_getc(SERIAL_UART_ID);

        if (symbol == '\n' || symbol == '\r' || (incomeIndex > SERIAL_MAX_INCOME_LEN - 1))
        {
            income[incomeIndex] = 0;
            incomeIndex = 0;
            return income;
        }
        income[incomeIndex] = symbol;
        incomeIndex++;
    }
    return 0;
}

/**
 * @brief Prints text to serial.
 * This differs from printf, that it checks if uart is writable.
 *
 * @param line      text to print.
 * @return true     printing succeeded.
 * @return false    printing failed, no serial available.
 */
bool serialSendLine(char *line)
{
    if (uart_is_writable(SERIAL_UART_ID))
    {
        printf("%s\n", line);
        return true;
    }
    return false;
}

/**
 * @brief Sets interrupt on serial activity with handler function.
 *
 * @param handlerFunction handler function.
 */
void serialSetInterruptHandler(void *handlerFunction)
{
    static int UART_IRQ = SERIAL_UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;

    irq_set_exclusive_handler(UART_IRQ, handlerFunction);
    irq_set_enabled(UART_IRQ, true);
    uart_set_irq_enables(SERIAL_UART_ID, true, false);
}