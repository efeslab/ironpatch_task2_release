/*
 * Bumper.cpp
 *
 * This class will be used to keep track of the bumper state and update
 * the corresponding bumper LEDs based on current state. This class has
 * members externally modified, such as brake_state, and state is updated
 * via method calls, such as turn_signal_routine.
 */

#include "Bumper.h"

Bumper::Bumper( TIM_HandleTypeDef *hardware_timer, volatile uint32_t *timer ) {
	/* Initialize all states to low/false */
	// Hardware timer reference
	hrtc_ref = hardware_timer;
	total_time = timer;
	// Bumper LED States
	outer_left = inner_left = inner_right = outer_right = LOW;
	// Brake related vars
	prev_brake_state = false;
	brake_state = false;
	flash_lock = false;
	num_flashes = 0;
	flash_timer = 0;
	// Turn signal vars
	need_to_signal = false;
	left_lock = right_lock = false;
	signal_timer = 0;
	signal = 0;
}

inline uint32_t Bumper::get_time() {
	return 0; // Marina klee emulate timer here
	return *total_time + __HAL_TIM_GET_COUNTER(hrtc_ref);
}


void Bumper::brake(void) {
	if (brake_state) {
		if (!flash_lock){ inner_left  = inner_right = HIGH; }
		if (!left_lock)    { outer_left  = HIGH; }
		if (!right_lock)   { outer_right = HIGH; }
	}
	else {
		inner_left = inner_right = LOW;
		if (!left_lock)  { outer_left = LOW; }
		if (!right_lock) { outer_right = LOW; }
		num_flashes = 0;
	}
}

void Bumper::brake_flash(void) {
    if ((get_time() - flash_timer) > (TicksPerSecond/4)){
        flash_timer = get_time();
        inner_left = inner_right = !inner_left;
        num_flashes++;
        if (num_flashes > 4) {
            //inner_left = inner_right = LOW;
            num_flashes = 0;
            flash_lock = false;
            flash_timer = 0;  // in case brakes need to flash again in < flash_timer
        }
    }
}

/*
 * 0000b = No Turn being signaled
 * 0001b = Left Turn to be Flashing
 * 0010b = Right turn to be Flashing
 * 0011b to 1101b = Reserved
 * 1110b = Error (to include both left and right selected simultaneously)
 * 1111b = Not available (do not change)
 */
void Bumper::turn_signal_routine() {
    if (get_time() - signal_timer > (TicksPerSecond/2)){
        if (signal == 0b1110){
            left_lock = right_lock = true;
            outer_left = outer_right = !(outer_left);  // Sync turn signals (left and right on reflect hazards)
            // This can unfortunately induce a 1 cycle delay on the left signal in the event
            // the right signal is already on, then left signal is turned on
        }
        else if (signal == 0b0001 ){
            left_lock = true;
            right_lock = false;
            outer_left = ! outer_left;
        }
        else if (signal == 0b0010){
            right_lock = true;
            left_lock = false;
            outer_right = ! outer_right;
        }
        else if (signal == 0b1111) {  }  // do nothing/continue behavior, node may be down
        else {
        	right_lock = left_lock = false;
        }
        signal_timer = get_time();
	}
}

void Bumper::brake_routine(void) {
	if (flash_lock) {
		brake_flash();
	}
	brake();  // applies brakes based on state
}
