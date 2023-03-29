#include "watchdog.h"

static int watchdog_fd;


/*
* @brief  Handle when we want to set system watchdog reset timeout
*
* @param  t: Timeout in seconds for system not responding
*/
void setwdtimeout(int t) {
  int timeout = t;
	int status = 0;
  // Don't be ridiculous
  if (timeout < 0) return;
	int fd = openat(AT_FDCWD, WATCHDOG_FILE_NAME, O_WRONLY|O_CLOEXEC);
	write(fd, "V", 1);
	status = ioctl(fd, WDIOC_SETTIMEOUT, &timeout);
	if (status < 0) {
		perror("setwdtimeout\n");
	}
	close(fd);
}


/*
* @brief  Open the watchdog file descriptor
*
*/
void wake_watchdog() {
    watchdog_fd = open(WATCHDOG_FILE_NAME, O_WRONLY);
    if (watchdog_fd < 0) {
        perror("Unable to open watchdog\n");
        watchdog_fd = -1;
    }
}


/*
* @brief  Write to watchdog, extending time
*
*/
void feed_watchdog() {
    int write_status;
    if (watchdog_fd > 0) {
        write_status = write(watchdog_fd, "\0", 1);
        if (write_status <= 0) {
            perror("Failed to feed watchdog\n");
        }
    }
}


/*
* @brief  Close watchdog file descriptor
*
*/
void rest_watchdog() {
    if (watchdog_fd > 0) {
        // write(watchdog_fd, &"V", 1); Once FD is closed watchdog is disabled, no need for magic stop
        if (close(watchdog_fd) < 0){
            perror("Error resting watchdog\n");
        }
    }
}