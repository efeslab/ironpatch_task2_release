#include "monitor.h"

// Marina fixup begin
SML_t SML;
// Marina fixup end

/**
  * @brief  Initializes the buffer used for storing SPN IDs to monitor
  *
  */
void initialize_spn_monitor() {
    SML.spns = (uint32_t*)malloc(sizeof(uint32_t) * 10);
    SML.size = 0;
    SML.on = 1;
}

/**
  * @brief  This function will monitor the SPNs active under the SPN monitor list, as well as performing certain actions
  *         based on a monitor control frame.
  *
  * @param  buff: The data buffer of the J1939 frame
  */
static void process_control_frame(unsigned char buff[]) {
    // Parse buffer for SPN addition or deletion
    uint8_t control = (uint8_t)buff[0];
    uint32_t spn = 0;
    memcpy(&spn, (buff + 4), 4);
    spn = __bswap_32 (spn);
    switch (control) {
        case 0: // Clear
            free(SML.spns);
            SML.spns = malloc(sizeof(uint32_t) * 10);
            SML.size = 0;
            printf("Cleared SPN list\n");
            return;
        case 1: // Add SPN to monitor list
            for (int sml_index = 0; sml_index < SML.size; sml_index++) {
                if (SML.spns[sml_index] == spn) {
                    return;  // already in list
                }
            }
            if (0 == ((SML.size + 1) % 10)) {
                SML.spns = (uint32_t*)realloc(SML.spns, sizeof(uint32_t) * (SML.size + 10));
            }
            SML.spns[SML.size] = spn;
            SML.size++;
            printf("Added SPN %i to list\n", spn);
            return;
        case 2: // Remove SPN from monitor list
            if (SML.size != 0) {
                // remove element
                for (int spn_index = 0; spn_index < SML.size; spn_index++) {
                    if (SML.spns[spn_index] == spn) {  // mem move removal
                        memmove(SML.spns + spn_index, SML.spns + (spn_index + 1), sizeof(SML.spns[0]) * (SML.size - spn_index - 1));
                        // element found, decrement size and adjust buffer size
                        SML.size--;
                        // Shrink array if needed
                        if ((SML.size % 10) == 0 && SML.size != 0) {
                            SML.spns = (uint32_t*)realloc(SML.spns, sizeof(uint32_t) * (SML.size));
                        }
                        printf("Removed SPN %i from list\n", spn);
                        return;
                    }
                }
            }
            return;
        case 3: // Silence monitor list
            SML.on = 0;
            printf("Silenced SPN monitor\n");
            return;
        case 4: // Activate monitor list
            SML.on = 1;
            printf("Activated SPN monitor\n");
            return;
        case 5: // enable all SPN monitoring
            SML.on = 2;
            printf("Full SPN monitoring activated\n");
            return;
        default:  // Reserved, invalid format
            printf("Monitor control %i not supported\n", control);
            return;
    }
}

/**
  * @brief  This function will monitor the SPNs active under the SPN monitor list, as well as performing certain actions
  *         based on a monitor control frame.
  *
  * @param  pgn: The PGN value taken from an incoming J1939 frame
  * @param  buff: The data buffer of the J1939 frame
  */
void monitor_spns(int f_sock, uint32_t pgn, unsigned char buff[]) {
    // Handle monitor control frames
    if (pgn == MONITOR_PGN) {
        process_control_frame(buff);
        return;
    }

    // Monitor is disabled
    if (SML.on == 0) {
        return;
    }

    // Attempt to resolve PGN
    pgn_fp fp;
    fp = fetch_pgn(pgn);
    if (fp == NULL) {
        return;
    }

    // Process PGN
    int size = 0;
    struct SPN_ID* spn_list = (*fp)(&size);
    struct SPN spn;
    __uint64_t spn_data = 0;
    __uint64_t tmp2 = 0;

    // create forwarding frame
    struct can_frame cframe;
    cframe.can_id = pgn << 8;
    cframe.can_dlc = 8;
    int monitor_detected = 0;
    memset(cframe.data, 0, 8);

    // Monitoring all SPNs
    if (SML.on == 2) {
        for (int spn_list_index = 0; spn_list_index < size; spn_list_index++) {
            // Query debug output if available
            spn = (*spn_list[spn_list_index].fp)(buff);
            free(spn.data);
        }
        // Forward Frame
        memcpy(cframe.data, buff, 8);
        if (write(f_sock, &cframe, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
            err(1, "Failed to forward data");
        }
    } else {

        // Iterate over all SPNs identified in incoming PGN
        for (int spn_list_index = 0; spn_list_index < size; spn_list_index++) {
            // Check if current SPN is in the monitor list
            for (int monitor_spn_index = 0; monitor_spn_index < SML.size; monitor_spn_index++) {
                if (spn_list[spn_list_index].id == SML.spns[monitor_spn_index]) {
                    monitor_detected = 1;
                    spn = (*spn_list[spn_list_index].fp)(buff);
                    // Properly cast type
                    if (spn.type == 0) {
                        spn_data = *(__uint8_t *)spn.data;
                    } else if (spn.type == 1) {
                        spn_data = *(__uint16_t *)spn.data;
                    } else if (spn.type == 2) {
                        spn_data = *(__uint32_t *)spn.data;
                    } else {
                        spn_data = *(__uint64_t *)spn.data;
                    }
                    // re-pack data into PGN
                    spn_data = spn_data << spn.position;
                    tmp2 = spn_data | tmp2;
                    free(spn.data);
                }
            }
        }
        // If a frame was detected in the monitor list, send the filtered frame
        if (monitor_detected == 1) {
            tmp2 = __bswap_64(tmp2);
            memcpy(cframe.data, &tmp2, sizeof(__uint64_t));
            if (write(f_sock, &cframe, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
                err(1, "Failed to forward data");
            }
        }
    }
    free(spn_list);
}
