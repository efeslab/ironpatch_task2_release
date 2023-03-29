/*
 * UnitTesting.h
 *
 */

#ifndef SRC_UNITTESTING_H_
#define SRC_UNITTESTING_H_
#define BrakeID 1
#define TurnID 2
#include <cstdint>
#include "stm32f0xx_hal.h"

class UnitTesting {
public:
	int iteration;
	volatile uint32_t *timer;
	uint32_t last_time;
	uint32_t last_seconds;
	TIM_HandleTypeDef *htim_ref;
	bool double_msg;

	UnitTesting( volatile uint32_t *time_ptr, TIM_HandleTypeDef *htim2_ptr );

	/*
	 * Takes the seconds in as an argument just bc it was easy.
	 * will perform arithmetic given seconds passed and internal iteration
	 * and provide a boolean on if a message is ready
	 */
	bool CheckInbox();  // Call to check if there's more inputs

	inline uint32_t get_time();

	/*
	 * Poor man's generator, will produce an output based on the internal iteration
	 */
	void RecvMsg(unsigned long *id, unsigned char *len, unsigned char buff[]);

	void RecvMsgBrakeOnly(unsigned long *id, unsigned char *len, unsigned char buff[]);

	void RecvMsgVulnTest(unsigned long *id, unsigned char *len, unsigned char buff[]);

	void RecvMsgTurnSignals(unsigned long *id, unsigned char *len, unsigned char buff[]);

};

#endif /* SRC_UNITTESTING_H_ */
