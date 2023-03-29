#ifndef SRC_GPIO_H_
#define SRC_GPIO_H_

#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "local_defs.h"

/* PWM Driver */
#define PWM_PERIOD 1000
#define PWM_LOW 10
#define PWM_HIGH 100
#define PWM_PERIOD_STR "1000"
#define PWM_LOW_STR "10"
#define PWM_HIGH_STR "100"


typedef struct pin_type {
    int state;
    int period;
    int duty_cycle;
    int polarity;
    int enable;

} pin_t;

void initialize_pins();

void close_pins();

pin_t get_pin(int pin);

void set_power(int pin, int state);

void set_state(int pin);

void enable_pin(int pin);

void disable_pin(int pin);

void polarity_normal(int pin);

void set_period(int pin, char * period, unsigned char len);

void get_period(int pin, char * period);

void set_duty(int pin, int state);

void get_duty(int pin, char * duty);

#endif /* SRC_GPIO_H_ */