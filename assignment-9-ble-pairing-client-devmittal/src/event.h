/*
 * event.h
 *
 * brief: Contains function definitions and macros related to event.c
 *
 *  Created on: 14-Sep-2019
 *      Author: Devansh
 */

#ifndef SRC_EVENT_H_
#define SRC_EVENT_H_

#include <stdint.h>

/* events */
enum My_Events {
	TURN_SI7021_ON = 1,
	DELAY_DONE,
	I2C_TRANSFER_DONE = 4,
	BUTTON_PRESSED = 8,
};

/* states */
enum My_States {
	STATE0_POWER_UP,
	STATE1_I2C_WRITE_START,
	STATE2_I2C_WRITE_DONE,
	STATE3_I2C_READ_START,
	STATE4_I2C_READ_DONE,
}current_state;

volatile uint32_t event;
uint8_t bond_status;

void eventInit(void);
void handle_event(void);
void CalculateTemp(uint8_t* read_data);
void handle_event_BT(uint32_t btEvent);

#endif /* SRC_EVENT_H_ */
