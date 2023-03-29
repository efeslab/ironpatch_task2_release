#ifndef SRC_MONITOR_H_
#define SRC_MONITOR_H_

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <linux/can.h>
#include <err.h>
#include <unistd.h>
#include <byteswap.h>
#include "spn.h"

#define MONITOR_PGN 0xFFEE
typedef struct conf_t {
    const char *intf;
    char *ranges;
    uint64_t name;
    // Current Source Address
    uint8_t current_sa;
    int sig_term;
    int sig_alrm;
    int state;
    struct can_frame claim_msg;
} Conf ;

// Marina: getting a compilation error for having this variable defined in the h file
/*
struct spn_monitor_list {
    uint32_t* spns;
    uint16_t size;  // Probably an outrageous size for insertion/removal methods
    uint8_t on;
} SML;
*/

// Marina fixup begin
typedef struct spn_monitor_list {
    uint32_t* spns;
    uint16_t size;  // Probably an outrageous size for insertion/removal methods
    uint8_t on;
} SML_t;
// Marina fixup end

extern SML_t SML;

void initialize_spn_monitor();

void monitor_spns(int f_sock, uint32_t pgn, unsigned char buff[]);

#endif