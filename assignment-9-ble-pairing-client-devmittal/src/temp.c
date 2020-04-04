/*
 * i2c.c
 *
 * brief: Functions related to i2c transfers for the temperature sensor
 *
 *  Created on: 13-Sep-2019
 *      Author: Devansh
 */

#include "i2cspm.h"
#include "em_i2c.h"
#include "temp.h"
#include "main.h"
#include "em_core.h"
#include "event.h"
#include "log.h"

/*
 * @func - I2C0_IRQHandler
 * @brief - Sets event on completion of i2c transfer
 * @parameters - none
 * @return - none
 */
void I2C0_IRQHandler(void)
{
	I2C_TransferReturn_TypeDef ret;

	ret = I2C_Transfer(I2C0);
	if(ret != i2cTransferInProgress)
	{
		//LOG_INFO("I2C Interrupt done. Ret: %d\n", ret);
		//CORE_DECLARE_IRQ_STATE;
		//CORE_ENTER_CRITICAL();
		//event |= I2C_TRANSFER_DONE;
		gecko_external_signal(I2C_TRANSFER_DONE);
		//CORE_EXIT_CRITICAL();
	}
}

/*
 * @func - i2cInit
 * @brief - init i2c
 * @parameters - none
 * @return - none
 */
void i2cInit(void)
{
	I2CSPM_Init_TypeDef i2c_parameters = {
			I2C0,						/* Use I2C instance 0 */
			gpioPortC,  				/* SCL port */
			10,                         /* SCL pin */
			gpioPortC,                  /* SDA port */
			11,                         /* SDA pin */
			14,                         /* Location of SCL */
			16,                         /* Location of SDA */
			0,                          /* Use currently configured reference clock */
			I2C_FREQ_STANDARD_MAX,      /* Set to standard rate  */
			i2cClockHLRStandard,        /* Set to use 4:4 low/high duty cycle */
	};

	NVIC_EnableIRQ(I2C0_IRQn);
	I2CSPM_Init(&i2c_parameters);
}

/*
 * @func - i2cTransfer
 * @brief - Initiate i2c read/write (non blocking)
 * @parameters - int - read or write
 * 				 uint8_t* - pointer to read/write data
 * 				 uint16_t - length of data
 * @return - void
 */
void i2cTransfer(int rw, uint8_t* data, uint16_t len)
{
	transfer_dets.addr = TEMP_ADDR << 1; //Set slave addr

	if(rw == I2C_WRITE)
	{
		transfer_dets.flags = I2C_FLAG_WRITE;
	}
	else
	{
		transfer_dets.flags = I2C_FLAG_READ;
	}

	transfer_dets.buf[0].data = data;
	transfer_dets.buf[0].len = len;

	I2C_TransferInit(I2C0, &transfer_dets);
}


