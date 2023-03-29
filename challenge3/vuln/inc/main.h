#ifndef SRC_MAIN_H_
#define SRC_MAIN_H_

#include <linux/can.h>
#include <linux/can/raw.h>
#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <sys/select.h>
#include <time.h>
#include <stdint.h>

#include "gpio.h"
#include "bumper.h"
#include "local_defs.h"

int init_can(int *sock, char *interface);

// Routine for handling incoming brake CAN frames, vulnerability exists here
void rx_brake_routine( unsigned char buff[], struct Bumper *bumper );
// Routine for handling incoming turn signal CAN frames
void rx_signal_routine( unsigned char buff[], struct Bumper *bumper );

#endif /* SRC_MAIN_H_ */