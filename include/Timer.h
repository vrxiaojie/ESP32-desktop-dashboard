#ifndef TIMER_H_
#define TIMER_H_

#include "Arduino.h"
#include "Mqtt.h"

extern uint16_t tim1_IRQ_count;

void timer_init(void);

#endif