#ifndef SERVO_H
#define SERVO_H

void servoInit(uint8_t servoPin);
void servoMoveToAngle(uint8_t servoPin, float degree);
void servoMoveUp(uint8_t servoPin);
void servoMoveDown(uint8_t servoPin);

#endif