#ifndef PWM_H
#define PWM_H

#include "types.h"

void pwm_init(u8 channel);
void pwm_set(u8 channel, u8 pos);

#endif