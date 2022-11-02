#ifndef SERVO_H
#define SERVO_H

#define SERVO_MAX_ANGLE 180

void servoSetup(uint8_t servoPin);
void servoMoveToAngle(uint8_t servoPin, float degree);

#endif