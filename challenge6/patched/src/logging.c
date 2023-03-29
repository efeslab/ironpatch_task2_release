#include "logging.h"

/* Global variables */
static uint32_t current_frame_count = 0;
static FILE * wfd = NULL;
static struct MBlock mblock = {
        . generation = "CAN2",
        . version = "TU2",
        . logger_number = "12",
        . file_number = "AMP",
        . micro_of_sdcard = "N/A"
};

/* Key used for the encryption routines */
static unsigned char KEY[KEY_SIZE];
/* IV used for encryption routines */
static unsigned char IV[IV_SIZE];


/**
  * @brief  Resets iterative values in the Mblock structure
  *
  */
void __attribute__((optimize("O0"))) reset_mblock(){
    /* Zero our Mblock fields */
    memset(mblock.frames, 0, BUFFER_SIZE*sizeof(struct can_frame));
    memset(mblock.time_stamps, 0, BUFFER_SIZE*sizeof(struct timeval));
    memset(mblock.integrity_ptr, 0, INTEGRITY_SIZE);
}


/**
  * @brief  Setup function for the logger, opens a file and initializes the encryption parameters
  *
  * @param  log_name:  Name of the log file that Mblocks should be logged to
  */
void logging_setup(char * log_name){
    /* Open File Descriptor */
     if ((wfd = fopen(log_name, "wb")) == NULL)
            perror("Could not create initial descriptor for %s");
    /* Initialize the tally counts */
    memset(mblock.rx_counts, 0, sizeof (mblock.rx_counts));
    memset(mblock.can_rx_err_counts, 0, sizeof (mblock.can_rx_err_counts));
    memset(mblock.can_tx_err_counts, 0, sizeof (mblock.can_tx_err_counts));
    mblock.integrity_ptr = malloc(INTEGRITY_SIZE);
    /* Setup and write Key and IV */
    initialize_write(KEY, KEY_SIZE);
    initialize_write(IV, IV_SIZE);
}


/**
  * @brief  Fills the crypto_param with RAND bytes and writes the param out to logging file
  *
  * @param  crypto_param: This is a crypto related value that needs to be randomized and written to a file
  * @param  size: Size of the crypto_param
  */
void initialize_write(unsigned char* crypto_param, int size) {
    if (RAND_bytes(crypto_param, size) != 1) {
        handleErrors();
    }
    /* Write out key length & key */
    if (fwrite(&size, 1, sizeof(int), wfd) < 1) {
        perror("Unable to write crypto param size to file");
    }
    if (fwrite(crypto_param, 1, size, wfd) < 1) {
        perror("Unable to write crypto param to file");
    }
}


/**
  * @brief  Serializes the Mblock, encrypts it, and writes it to a file.
  *
  */
void write_encrypted(void) {
    /* Update integrity check value */
    uint32_t mblock_size = 0;

    unsigned char *mblock_data = serialize(&mblock_size);
    unsigned int digest_length;
    unsigned char *buffer = malloc (mblock_size);
    digest_message(mblock_data, mblock_size - INTEGRITY_SIZE, &buffer, &digest_length);
    if (digest_length != INTEGRITY_SIZE) {
        handleErrors();
    }
    memcpy(mblock.integrity_ptr, buffer, digest_length);
    free(buffer);

    free(mblock_data);
    /* serialize mblock data before write */
    unsigned char *plaintext = serialize(&mblock_size);
    /* Allocate buffer for returning ciphertext, plus some extra bytes for block alignment */
    unsigned char *ciphertext = malloc(mblock_size + EXTRA_ENC_BUFF);
    /* Pass plaintext to encryption algorithm */
    int bytes_written = 0;
    bytes_written = encrypt(plaintext, mblock_size, KEY, IV, ciphertext);
    /* Write Mblock Length:Mblock */
    if (fwrite(&bytes_written, sizeof(int), 1, wfd) < 1) {
        perror("Could not write tag length");
    }
    if (fwrite(ciphertext, 1, bytes_written, wfd) < 1) {
        perror("Could not write ciphertext of length");
    }
    free(plaintext);
    free(ciphertext);
    initialize_write(IV, IV_SIZE);
}

/**
  * @brief  Logging Handler, takes a CAN frame, and proceeds to store the frame in the Mblock structure
  *         and writing to file if full
  *
  * @param  read_frame: The incoming can frame to be logged
  */
void logging_handler(struct can_frame read_frame) {
    memcpy(&mblock.frames[current_frame_count], &read_frame, sizeof(struct can_frame));
    gettimeofday(&mblock.time_stamps[current_frame_count], NULL);

    current_frame_count++;
    mblock.rx_counts[0]++;
    if (current_frame_count >= BUFFER_SIZE){
        /* Dump into log */
        write_encrypted();

        if (FLUSH_AT_RUNTIME == 1) {
            if (fflush(wfd)) {
                perror("Could not flush CAN data log");
            }
        }
        /* Refresh */
        reset_mblock();
        current_frame_count = 0;
    }
}


unsigned char* __attribute__((optimize("O0"))) serialize(uint32_t* mblock_size) {
    uint32_t tmp_size = 0;
    tmp_size = tmp_size + sizeof(mblock.generation) + sizeof(mblock.frames) + sizeof(mblock.time_stamps)
     + sizeof(mblock.rx_counts) + sizeof(mblock.can_rx_err_counts) + sizeof(mblock.can_tx_err_counts)
     + sizeof(mblock.version) + sizeof(mblock.logger_number) + sizeof(mblock.file_number)
     + sizeof(mblock.micro_of_sdcard) + INTEGRITY_SIZE;

    unsigned char *data = malloc(tmp_size);
    *mblock_size = tmp_size;
    uint32_t c_size = 0;

    memcpy(data + c_size, mblock.generation, sizeof(mblock.generation));
    c_size = c_size + sizeof(mblock.generation);

    memcpy(data + c_size, mblock.frames, sizeof(mblock.frames));
    c_size = c_size + sizeof(mblock.frames);

    memcpy(data + c_size, mblock.time_stamps, sizeof(mblock.time_stamps));
    c_size = c_size + sizeof(mblock.time_stamps);

    memcpy(data + c_size, mblock.rx_counts, sizeof(mblock.rx_counts));
    c_size = c_size + sizeof(mblock.rx_counts);

    memcpy(data + c_size, mblock.can_rx_err_counts, sizeof(mblock.can_rx_err_counts));
    c_size = c_size + sizeof(mblock.can_rx_err_counts);

    memcpy(data + c_size, mblock.can_tx_err_counts, sizeof(mblock.can_tx_err_counts));
    c_size = c_size + sizeof(mblock.can_tx_err_counts);

    memcpy(data + c_size, mblock.version, sizeof(mblock.version));
    c_size = c_size + sizeof(mblock.version);

    memcpy(data + c_size, mblock.logger_number, sizeof(mblock.logger_number));
    c_size = c_size + sizeof(mblock.logger_number);

    memcpy(data + c_size, mblock.file_number, sizeof(mblock.file_number));
    c_size = c_size + sizeof(mblock.file_number);

    memcpy(data + c_size, mblock.micro_of_sdcard, sizeof(mblock.micro_of_sdcard));
    c_size = c_size + sizeof(mblock.micro_of_sdcard);

    void * (*memcpy_ptr)(void *, const void *, size_t) = &memcpy;  /* Stop call optimization */
    (*memcpy_ptr)(data + c_size, mblock.integrity_ptr, INTEGRITY_SIZE);
    return data;
}


/**
  * @brief  Call on program exit, will close file properly, ensuring all data is logged out
  *
  */
void terminate_logging_gracefully() {
    /* Check if we have frames to write out */
    if (current_frame_count > 0) {
        write_encrypted();
    }
    /* fclose will also flush */
    free(mblock.integrity_ptr);
    fclose(wfd);
}
