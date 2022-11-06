#ifndef SERIAL_H
#define SERIAL_H

#define SERIAL_MAX_INCOME_LEN 256

void serialUartInit();
char *serialUartGetLastLine();
char *serialUsbGetLastLine();
bool serialUartSendLine(char *line);
void serialUartSetInterruptHandler(void *handlerFunction);

#endif