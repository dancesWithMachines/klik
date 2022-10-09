#ifndef SERIAL_H
#define SERIAL_H

#define SERIAL_MAX_INCOME_LEN 256

void serialInit();
char *serialGetLastLine();
bool serialSendLine(char *line);

#endif