//
// Created by subhojeet on 7/6/21.
//

#ifndef TRANSPORTLAYERISSUES_TRANSPORT_H
#define TRANSPORTLAYERISSUES_TRANSPORT_H


#include "local_defs.h"
#include <unistd.h>
#include <byteswap.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <stdio.h>
#include <err.h>


typedef enum {
    IDLE,
    EST
} State;

#define SRC 0x49
#define TPCM_MAGIC 0x00ec0000
#define TPCM_MSG 0x18ECFF00
#define TPDT_MAGIC 0x00eb0000
#define RQST_MAGIC 0x00ea0000
#define TP_CM_RTS 0x10         // Transport Protocol Communication Management Request to Send
#define TP_CM_CTS 0x11         // Transport Protocol Communication Management Clear to Send
#define TP_CM_ABORT 0xFF       // Transport Protocol Communication Management Abort
#define SRC_MASK 0x000000ff
#define DST_MASK 0x0000ff00
#define NUM_ALLOWABLE_CONNECTIONS 10

/* Globals */
struct ConnectionInfo{
    State state;
    uint8_t num_packets;
    uint8_t recv_num_packets;
    uint8_t* data;
};


void transport_handler(struct can_frame , int , uint8_t );


/* Main functions */
void transport_setup();


void transport_takedown();

#endif //TRANSPORTLAYERISSUES_TRANSPORT_H
