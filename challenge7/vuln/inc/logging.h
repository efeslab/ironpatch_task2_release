#ifndef AMP_CHALLENGE_06_CRC_H
#define AMP_CHALLENGE_06_CRC_H

#include "conf.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <linux/can.h>
#include <sys/time.h>
#include <stdint.h>
#include "EVP_aes_256_cbc.h"

struct MBlock{
    char generation[4];
    struct can_frame frames[BUFFER_SIZE];
    struct timeval time_stamps[BUFFER_SIZE];
    uint32_t rx_counts[3];
    uint8_t can_rx_err_counts[3];
    uint8_t can_tx_err_counts[3];
    char version[3];
    char logger_number[2];
    char file_number[3];
    char micro_of_sdcard[3];
    unsigned char *integrity_ptr;
};

#define INTEGRITY_SIZE 32
#define KEY_SIZE 32
#define IV_SIZE 16
#define EXTRA_ENC_BUFF 16

/* Functions */
void logging_setup(char * log_name);
void logging_handler(struct can_frame read_frame);
void terminate_logging_gracefully();
void log_to_file();
void reset_mblock();
void initialize_write(unsigned char * crypto_param, int size);
void write_encrypted(void);
unsigned char* serialize(uint32_t *mblock_size);

#endif /*AMP_CHALLENGE_06_CRC_H */