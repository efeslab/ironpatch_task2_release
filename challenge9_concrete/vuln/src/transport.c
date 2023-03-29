//
// Created by subhojeet on 7/6/21.
//
#include "transport.h"
#include <assert.h> // Marina


/* Globals */
static struct ConnectionInfo* connection_infos[256];
static uint8_t src = SRC;
static struct can_frame CTS;
static uint8_t num_packets;
static uint8_t num_connections = 0;


static void delete_connection(){
    if (connection_infos[src]->data != NULL){
        free(connection_infos[src]->data);
        connection_infos[src]->data = NULL;
    }
    free(connection_infos[src]);
    connection_infos[src] = NULL;
    num_connections--;
}


static uint8_t create_conn(uint16_t size){
	// Marina: patch begin
	assert(num_connections < 254);
	// Marina: patch ends
    if (connection_infos[src] == NULL){
        connection_infos[src] = (struct ConnectionInfo *)calloc(1, sizeof(struct ConnectionInfo));
        connection_infos[src]->state = IDLE;
        connection_infos[src]->data = NULL;
    }
    if ((size % 7) != 0) {
            size = size + 7 - (size % 7);
    }
    // Vulnerable RTS data handling
    if (connection_infos[src]->data == NULL){
        connection_infos[src]->data = (uint8_t *)calloc(size, sizeof(uint8_t));
    }
    else{
        connection_infos[src]->data = (uint8_t *) realloc(connection_infos[src]->data , size);
    }
    num_connections++;
    return 1;
}


void transport_handler(struct can_frame read_frame, int can_socket_desc, uint8_t current_sa){
    uint16_t size = 0;
    // Get the sender
    src = read_frame.can_id & SRC_MASK;
    if (((read_frame.can_id & DST_MASK) >> 8) == current_sa) {
        switch (read_frame.can_id & PF_MASK){
            case TPCM_MAGIC:  // TP.CM
                switch (read_frame.data[0]) {
                    case TP_CM_RTS: // RTS recv, CTS send
                        // Get message size in bytes
                        memcpy(&size, &read_frame.data[1], 2);
                        size = bswap_16(size);
                        // Get number of packets
                        num_packets = read_frame.data[3];
                        //printf ("Recieved RTS from %d to allocate %d bytes of data from %d no. of packets\n", src, size, num_packets);

                        // Create a session
                        if (create_conn(size) == 1){
                            connection_infos[src]->state = EST;
                            connection_infos[src]->num_packets = num_packets;
                            
                            CTS.can_id = (CTS.can_id & 0xffff00ff) | (src << 8);
                            CTS.data[1] = read_frame.data[4];
                            memcpy (&CTS.data[5], &read_frame.data[5], 3); 
                            if (write(can_socket_desc, &CTS, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
                                err(1, "could not send CTS");
                            }
                        }
                        break;
                    case TP_CM_ABORT:
                        if (connection_infos[src] == NULL)
                            break;
                        printf ("Recieved Abort from %d\n", src);
                        delete_connection();
                        break;
                        
                }
                break;
            case TPDT_MAGIC:  // Data recieve
                if (connection_infos[src] == NULL)
                    break;

                if (connection_infos[src]->state != EST)
                    break;
                connection_infos[src]->recv_num_packets++;
                printf ("Recieved packet %d from SA %02x\n", connection_infos[src]->recv_num_packets, src);
                if (read_frame.data[0] == 0) break; // keep from some negative index

                // Copy bus data into connection_infos buffer, challenge 07 fix
                if (read_frame.data[0] > connection_infos[src]->num_packets){
                    printf("Arbit packet location detected! Connected closed\n");
                    delete_connection();
                    break;
                }
                memcpy(&connection_infos[src]->data[(read_frame.data[0] -1)*7], &read_frame.data[1], 7);

                if (connection_infos[src]->recv_num_packets >= connection_infos[src]->num_packets){
                    printf ("Recieved all %d packets, closing connection\n", connection_infos[src]->recv_num_packets);
                    delete_connection();
                }

                break;
        }
    }
}
            

/* Main functions */
void transport_setup(){
    // Init default CTS
    CTS.can_dlc = 8;
    CTS.can_id = TPCM_MSG | SRC | CAN_EFF_FLAG;
    memset(CTS.data, 0xff, 8);
    CTS.data[0] = TP_CM_CTS;
    CTS.data[2] = 1;
}


void transport_takedown(){
    for (int i = 0; i < 256; i++){
        if (connection_infos[i] != NULL){
            src = i;
            delete_connection();
        }
    }
}
