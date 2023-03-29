#include "main.h"

extern struct Bumper decoy_bumper; // Marina
extern int nr_iterations; // Marina


int main(int argc, char * argv[]){
    // Check parameters
    if (argc < 2) {
        printf("Usage %s [can_interface]\n", argv[0]);
        return 1;
    }

    // Setup logging
    FILE *log_file;
    log_file = fopen("/tmp/program_c.txt", "w");

    // Init CAN Socket
    char iter_name[10];
    snprintf(iter_name, sizeof(iter_name), "%s", argv[1]);
    int s = socket (PF_CAN, SOCK_RAW, CAN_RAW);
    while (init_can(&s, iter_name) != 0) {
        printf("Failed to create socket\n");
        sleep(3000);
    }
    printf("Socket created\n");

    // Init GPIO
    initialize_pins();
    for (int pin = 1; pin < 5; pin++) {
        set_period(pin, PWM_PERIOD_STR, sizeof(PWM_PERIOD_STR));
        set_duty(pin, PWM_LOW);
        set_power(pin, 1);
    }

    // Keep track of state
    struct Bumper* bumper;
    //bumper = (struct Bumper*) malloc(sizeof(struct Bumper)); // Marina: KLEE messes up memory allocations
    bumper = &decoy_bumper;
    init_bumper(bumper);

    // Variables for main loop
    int nfds = s + 1;
    fd_set  readfds;
    struct timeval timeout;
    struct can_frame cf;
    ssize_t nbytes;
    time_t now;

    for(int i = 0 ; i < nr_iterations ; i ++) { // Marina: make this paramenter
    //while (1) { // Marina: changed amount of iterations
        FD_ZERO(&readfds);
        FD_SET(s,&readfds);

        // Our tick if not ready
        timeout.tv_sec = 0;
        timeout.tv_usec = 50;  // milliseconds

        int ready = select(nfds, &readfds, NULL, NULL, &timeout);
        if ( ready > 0 ){
            nbytes = read(s, &cf, sizeof(struct can_frame));

            if (nbytes < 0 ) {
                perror("CAN read");
                return 1;
            }

            // Can_ID contains the first 3 bits, so get rid of them
            cf.can_id &= 0x1fffffff;
            time(&now);
            fprintf(log_file, "%s [%s]: ID:%08X DATA:%02X%02X%02X%02X%02X%02X%02X%02X\n",
                ctime(&now), "CAN", cf.can_id,
                 cf.data[0], cf.data[1], cf.data[2], cf.data[3],
                 cf.data[4], cf.data[5], cf.data[6], cf.data[7]);
            if (((cf.can_id >> 8) & 0x7FFFF) == PGN_CruiseControlVehicleSpeed1 ){
                rx_brake_routine(cf.data, bumper);
            }

            if (((cf.can_id >> 8) & 0x7FFFF) == PGN_OperatorsExternalLightControls){
                rx_signal_routine(cf.data, bumper);
            }
        }
        turn_signal_routine(bumper);
        brake_routine(bumper);
        // GPIO toggle
        set_duty(1, ((bumper->outer_left != 0)  ? PWM_HIGH : PWM_LOW) );
        set_duty(2, (bumper->inner_left != 0)  ? PWM_HIGH : PWM_LOW );
        set_duty(3, (bumper->inner_right != 0) ? PWM_HIGH : PWM_LOW );
        set_duty(4, (bumper->outer_right != 0) ? PWM_HIGH : PWM_LOW );
        printf("[%s] [%s] [%s] [%s]\r",
            (bumper->outer_left) ? "o" : "x",
            (bumper->inner_left) ? "o" : "x",
            (bumper->inner_right) ? "o" : "x",
            (bumper->outer_right) ? "o" : "x"
        );
    }

}


/**
  * @brief  Handles the initialization of the CAN socket. It applies a filter and binds the socket.
  *
  * @param  sock: a pointer to a socket, that will be initialized
  * @retval 0 on successful bind, -1 on failure
  */
int init_can( int *sock, char *interface ) {
    struct ifreq ifr;
    struct sockaddr_can addr;
    struct can_filter rfilter[2];

    strcpy(ifr.ifr_name, interface);
    ioctl(*sock, SIOCGIFINDEX, &ifr);  // Retrieve the interface index of the interface into ifr_ifindex

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    // Filter setup
    rfilter[0].can_id   = 0x00FEF100;
    rfilter[0].can_mask = 0x00FFFF00;
    rfilter[1].can_id   = 0x00FDCC00;
    rfilter[1].can_mask = 0x00FFFF00;

    setsockopt(*sock, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));

    // We're going to be listening
    if (bind(*sock,(struct sockaddr *)&addr, sizeof(addr)) < 0) {
        return -1;
    }
    return 0;
}

void rx_brake_routine( unsigned char buff[], struct Bumper *bumper ) {
    int16_t speed_value;  // vulnerability here
    uint8_t brake_switch;
    uint8_t prev_brake_state = bumper->brake_state;
    // Extract Speed and brake switch from frame
    speed_value  = (buff[3] << 8) + buff[2];  // buf[3] = speed integer, buf[2] = speed decimal
    brake_switch = (buff[4] & 0b00001100) >> 2;
    // update related bumper members
    bumper->brake_state = (brake_switch) ? 1 : 0;

    // This segment would ideally be moved to bumper method
    if (bumper->brake_state) {
        // Can also make speed != 0 here
        if ((speed_value > 0) && (!bumper->has_flashed)){  // speed > 0 and brakes were off last
            bumper->flash_lock = 1;
        }
    }
    else {
        bumper->flash_lock = 0;
        bumper->has_flashed = 0;
    }
}

void rx_signal_routine( unsigned char buff[], struct Bumper *bumper ) {
    bumper->signal = (buff[1] >>4);
}
