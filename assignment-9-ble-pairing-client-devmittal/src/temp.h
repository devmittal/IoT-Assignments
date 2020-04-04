/*
 * i2c.h
 *
 * brief: Funnction declarations and macros for i2c temperature sensor
 *
 *  Created on: 13-Sep-2019
 *      Author: Devansh
 */

#ifndef SRC_TEMP_H_
#define SRC_TEMP_H_

#include "em_i2c.h"

#define TEMP_ADDR 0x40
#define READ_TEMP_NO_HOLD_MASTER 0xF3 //Read temp addr

/* Formula to calculate temperature in celsius (taken from datasheet)*/
#define CAL_TEMP_IN_C(temp_code) \
	(((175.72) * (temp_code))/(65536)) - (46.85)

/* Structure variable passed to i2c_transferinit */
I2C_TransferSeq_TypeDef transfer_dets;

uint8_t read_data[2];
uint8_t write_data[1];

void i2cInit(void);
void i2cTransfer(int rw, uint8_t* data ,uint16_t len);


#endif /* SRC_TEMP_H_ */
