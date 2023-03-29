#ifndef WATCHDOG_H
#define WATCHDOG_H
#define WATCHDOG_FILE_NAME "/dev/null"
// Left as /dev/null for now, to avoid shutdowns during vuln testing

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <linux/watchdog.h>
#include <stdlib.h>
#include <sys/ioctl.h>

void setwdtimeout(int t);
void wake_watchdog();
void feed_watchdog();
void rest_watchdog();

#endif // WATCHDOG_H
