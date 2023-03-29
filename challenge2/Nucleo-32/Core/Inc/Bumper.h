/*
 * Contains hardware timer references, LED state variables, brake
 * and turn signal related variables.
 */

#ifndef SRC_BUMPER_H_
#define SRC_BUMPER_H_
#include "local_defs.h"     // Project defines
#include "stm32f0xx_hal.h"  // For HAL TIM function and types
//#include <cstdint>

class Bumper {
public:
	// Hardware timer
	TIM_HandleTypeDef *hrtc_ref;
	volatile uint32_t *total_time;
	// Bumper LED States
	bool outer_left, inner_left, inner_right, outer_right;
	// Brake related variables
	bool prev_brake_state;
	bool brake_state;
	bool flash_lock;
	uint32_t flash_timer;
	int num_flashes;
	// Turn Signal related variables
	bool need_to_signal;
	bool left_lock, right_lock;  // used for giving priority to turn signal > brakes
	uint32_t signal_timer;
	uint8_t signal;  // Raw bits transfered over the network

	Bumper( TIM_HandleTypeDef *hardware_timer, volatile uint32_t *timer );

	// Method to shorten HAL Hardware counter call
	inline uint32_t get_time();

	// Turns brake lights on while mindful of locks (flash, right, left lock)
	void brake( void );

	void brake_flash( void );

	void turn_signal_routine( void );

	void brake_routine( void );

};

#endif /* SRC_BUMPER_H_ */
