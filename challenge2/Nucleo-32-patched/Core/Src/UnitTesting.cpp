/*
 * UnitTesting.cpp
 *
 * If CAN network is unavailable, the polling, receive, and PGN checking can
 * be exchanged with the methods of this class. This is then used to simulate
 * CAN bus traffic and observe proper bumper functionality. This may be changed
 * to a loopback approach, where CAN frames are actually transmitted and received
 */

#include "UnitTesting.h"

UnitTesting::UnitTesting( volatile uint32_t *time_ptr, TIM_HandleTypeDef *htim_second_ref) {
	iteration = 0;
	timer = time_ptr;
	htim_ref = htim_second_ref;
	last_time = get_time();
	last_seconds = *time_ptr;
	double_msg = false;
}

bool UnitTesting::CheckInbox() {
	if (last_seconds - *timer){ // at least 1 second has passed
		last_seconds = *timer;
		return true;
	}
	return false;

}

inline uint32_t UnitTesting::get_time(){
	return *timer + __HAL_TIM_GET_COUNTER(htim_ref);
}


// method for just demonstrating typical use case behavior
void UnitTesting::RecvMsg(unsigned long *id, unsigned char *len, unsigned char buff[]) {
	buff[0] = buff[1] = buff[2] = buff[3] = buff[4] = buff[5] = buff[6] = buff[7] = 0x00;  // clear buff
	*len = 8;  // buff length is 8
	/*
	 * iteration 0;      Do nothing
	 * iteration 1;      Press brakes
	 * iteration 2;      Do nothing
	 * iteration 3-4;    Press brakes, left turn signal on
	 * iteration 5;      Do nothing
	 * iteration 6;      Right turn signal on
	 * iteration 7-8;    Press brakes, speed = 50;
	 * iteration 9;      Do nothing
	 * iteration 10-11;  hazards on
	 * iteration 12-13;  Brakes on, speed = 50, hazards still on
	 * iteration 14;     Do nothing
	 */
	// We will be forcibly changing the last_seconds to push two messages at very close time frames
	switch (iteration) {
	case 1:  // Press brakes
		*id = BrakeID;
		buff[4] = 0b00000100;  // brake on
		break;
	case 3:  // Press brakes, left signal on
	case 4:
		if (!double_msg) {
			*id = BrakeID;
			buff[4] = 0b00000100;  // brake on
			last_seconds = 0;  // Trick CheckInbox()
			double_msg = true;
			return;  // Don't increment iteration, double message
		} // else
		*id = TurnID;
		buff[1] = 0b00010000;  // left signal on
		double_msg = false;  // reset double message state
		break;
	case 6:  // Right signal
		*id = TurnID;
		buff[1] = 0b00100000;  // right signal on
		break;
	case 7:  // Brake, speed = 50, right signal
	case 8:
		*id = BrakeID;
		buff[4] = 0b00000100;  // brake on
		buff[3] = 0x32;        // speed = 50
		break;
	case 10:  // Hazards
	case 11:  // turn signals persist till next message updating change, technically only need case 10:
		*id = TurnID;
		buff[1] = 0b11100000;
		break;
	case 12:  // Brake, speed = 50, hazards
	case 13:
		*id = BrakeID;
		buff[4] = 0b00000100;  // brake on
		buff[3] = 0x32;        // speed = 50
		break;
	default:
		// Default statement sends both IDs with buff[0-7] = 0x00
		if (!double_msg){
			*id = BrakeID;
			last_seconds = 0;
			double_msg = true;
			return;
		}
		*id = TurnID;  // clear Brake message
		double_msg = false;
		break;

	}
	iteration++;
	iteration = iteration % 15;
}


// Method for testing just the brakes, iterates through main testing points (not edge cases)
void UnitTesting::RecvMsgBrakeOnly(unsigned long *id, unsigned char *len, unsigned char buff[]) {
	buff[0] = buff[1] = buff[2] = buff[3] = buff[4] = buff[5] = buff[6] = buff[7] = 0x00;  // clear buff
	*len = 8;  // buff length is 8
	*id = BrakeID;  // debugging brake signal ID
	/*
	 * iteration 0;   Do nothing
	 * iteration 1-2; Brakes Pressed, speed = 0
	 * iteration 3;   Do nothing
	 * iteration 4-5; Brakes Pressed, 0 < speed < 127
	 * iteration 6;   Do nothing
	 * iteration 7-8; Brakes Pressed, 127 < speed < 255
	 * iteration 9;   Do nothing
	 * iteration 10;  Brakes Pressed, speed = 0  // tests if flashing only occurs when brakes were previously released
	 * iteration 11;  Brakes Pressed, 0 < speed < 127
	 * iteration 12;  Do nothing
	 */
	switch (iteration) {
	case 1:
	case 2:
		buff[4] = 0b00000100;  // brake on
		break;
	case 4:
	case 5:
		buff[4] = 0b00000100;  // brake on
		buff[3] = 0x7e;        // Speed = 126
		break;
	case 7:
	case 8:
		buff[4] = 0b00000100;  // brake on
		buff[3] = 0xff;        // Speed = 255
		buff[2] = 0xff;
		break;
	case 11:
		buff[3] = 0x7e;        // Speed = 126
	case 10:
		buff[4] = 0b00000100;  // brake on
		break;
	default:
		break;  // Do nothing, buff is all 0x00 and ID is already set
	}
	iteration++;
	iteration = iteration % (13);  // all test cases gone though
}


// Method for testing the vulnerability in the brake flash
void UnitTesting::RecvMsgVulnTest(unsigned long *id, unsigned char *len, unsigned char buff[]) {
	buff[0] = buff[1] = buff[2] = buff[3] = buff[4] = buff[5] = buff[6] = buff[7] = 0x00;  // clear buff
	*len = 8;  // buff length is 8
	*id = BrakeID;  // debugging brake signal ID
	/*
	 * iteration 0;   Do nothing
	 * iteration 1-2; Brakes Pressed, speed < 127
	 * iteration 3;   Do nothing
	 * iteration 4-5; Brakes Pressed, speed > 127
	 * iteration 6;   Do nothing
	 */
	switch (iteration) {
	case 1:
	case 2:
		buff[4] = 0b00000100;  // brake on
		buff[3] = 0x0f;
		break;
	case 4:
	case 5:
		buff[4] = 0b00000100;  // brake on
		buff[3] = 0xff;        // Speed = 126
		break;
	default:
		break;  // Do nothing, buff is all 0x00 and ID is already set
	}
	iteration++;
	iteration = iteration % (7);  // all test cases gone though
}


// Method for testing just the turn signals, iterates through main testing points (not edge cases)
void UnitTesting::RecvMsgTurnSignals(unsigned long *id, unsigned char *len, unsigned char buff[]) {
	buff[0] = buff[1] = buff[2] = buff[3] = buff[4] = buff[5] = buff[6] = buff[7] = 0x00;  // clear buff
	*len = 8;  // buff length is 8
	*id = TurnID;  // Debugging turn signal ID
	/*
	 * iteration 0; Do nothing
	 * iteration 1-2; Left signal on
	 * iteration 3; Do nothing
	 * iteration 4-5; Right signal on
	 * iteration 6; Do nothing
	 * iteration 7-8; Both signals on
	 * iteration 9; Do nothing
	 */
	switch(iteration) {
	case 1:
	case 2:
		buff[1] = 0b00010000;  // left signal
		break;
	case 4:
	case 5:
		buff[1] = 0b00100000;  // right signal
		break;
	case 7:
	case 8:
		buff[1] = 0b11100000;  // hazards
		break;
	default:
		break;  // Do nothing, buff is all 0x00 and ID is already set
	}
	iteration++;
	iteration = iteration % (10);  // all test cases gone though
}
