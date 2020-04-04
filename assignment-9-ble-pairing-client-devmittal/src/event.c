/*
 * event.c
 *
 * brief: Contains the state machine to determine current state actions and next state.
 *
 * references: Conversion of temp to compatible format to transmit, adapted from the soc-thermometer example by SiLabs
 *
 *  Created on: 14-Sep-2019
 *      Author: Devansh
 */

#include "event.h"
#include "temp.h"
#include "main.h"
#include "log.h"
#include "gpio.h"
#include "letimer.h"
#include "em_core.h"
#include "gatt_db.h"
#include "infrastructure.h"
#include "display.h"
#include "gecko_ble_errors.h"

/*
 * @func - eventInit
 * @brief - Initialize variables/enums used in events
 * @parameters - none
 * @return - none
 */
void eventInit(void)
{
	event = 0;
	current_state = STATE0_POWER_UP; //First default state
}

/*
 * @func - CalculateTemp
 * @brief - Calculate temperature based on formula in ds
 * @parameters - read_data - pointer to location containing the raw temp read from sensor
 * @return - none
 */
void CalculateTemp(uint8_t* read_data)
{
	uint16_t temp_code, temp_code_msb;
	float temp;
	uint8_t htmTempBuffer[5]; /* Stores the temperature data in the Health Thermometer (HTM) format. */
	uint8_t flags = 0x00;   /* HTM flags set as 0 for Celsius, no time stamp and no temperature type. */
	uint8_t *p = htmTempBuffer; /* Pointer to HTM temperature buffer needed for converting values to bitstream. */
	uint32_t temperature;   /* Stores the temperature data read from the sensor in the correct format */
	uint8_t button_state_buffer[1];
	uint8_t *button_state = button_state_buffer;
	uint32_t button_value;
	uint32_t button;

	UINT8_TO_BITSTREAM(p, flags);

	temp_code_msb = read_data[0] << 8;
	temp_code = temp_code_msb | read_data[1];

	temp = CAL_TEMP_IN_C(temp_code);

	displayPrintf(DISPLAY_ROW_TEMPVALUE, "Temperature: %.2f C", temp);
	displayUpdate();

	/* Convert sensor data to correct temperature format. Temp should be in milli C */
	temperature = FLT_TO_UINT32((temp*1000), -3);
	/* Convert temperature to bitstream and place it in the HTM temperature data buffer (htmTempBuffer) */
	UINT32_TO_BITSTREAM(p, temperature);

	/* Send indication of the temperature in htmTempBuffer to all "listening" clients.
	* This enables the Health Thermometer in the Blue Gecko app to display the temperature.
	*  0xFF as connection ID will send indications to all connections. */
	gecko_cmd_gatt_server_send_characteristic_notification(
			0xFF, gattdb_temperature_measurement, 5, htmTempBuffer);

	LOG_INFO("Temperature: %.2f C\n", temp);
}

/*
 * @func - handle_event
 * @brief - Switch to states and assign next state based on event
 * @parameters - uint32_t  - event
 * @return - none
 */
void handle_event(void)
{
	CORE_DECLARE_IRQ_STATE;
	enum My_States next_state;
	uint8_t read_data[2];
	uint8_t write_data[1];
	//LOG_INFO("Event: %x\n", event);

	switch(current_state)
	{
		case STATE0_POWER_UP:
			/* State to power up temperature sensor and start 80ms timer */
			if((event & TURN_SI7021_ON) == TURN_SI7021_ON)
			{
				CORE_ENTER_CRITICAL();
				event &= ~TURN_SI7021_ON;
				CORE_EXIT_CRITICAL();
				//gpioEn(TEMP_SENSOR_ENABLE_port, TEMP_SENSOR_ENABLE_pin); //Load power management on
				timerSetEventInMS(DELAY_AFTER_SENSOR_POWER_UP_MS); //80ms delay
				next_state = STATE1_I2C_WRITE_START;
			}
			break;

		case STATE1_I2C_WRITE_START:
			/* State to start i2c write to temp sensor */
			if((event & DELAY_DONE) == DELAY_DONE)
			{
				CORE_ENTER_CRITICAL();
				event &= ~DELAY_DONE;
				CORE_EXIT_CRITICAL();
				sleepBlockStart(sleepEM2); //Start blocking from em2 for i2c
				write_data[0] = READ_TEMP_NO_HOLD_MASTER;
				i2cTransfer(I2C_WRITE, write_data, sizeof(write_data)); //write 0xF3 (temp)
				next_state = STATE2_I2C_WRITE_DONE;
			}
			break;

		case STATE2_I2C_WRITE_DONE:
			/* State to start 10ms delay between i2c write and read */
			if((event & I2C_TRANSFER_DONE) == I2C_TRANSFER_DONE)
			{
				CORE_ENTER_CRITICAL();
				event &= ~I2C_TRANSFER_DONE;
				CORE_EXIT_CRITICAL();
				sleepBlockEnd(sleepEM2); //Stop blocking at end of i2c transfer
				timerSetEventInMS(DELAY_BETWEEN_WRITE_READ_MS); //10ms delay
				next_state = STATE3_I2C_READ_START;
			}
			break;

		case STATE3_I2C_READ_START:
			/* state to start i2c read from temp sensor */
			if((event & DELAY_DONE) == DELAY_DONE)
			{
				CORE_ENTER_CRITICAL();
				event &= ~DELAY_DONE;
				CORE_EXIT_CRITICAL();
				sleepBlockStart(sleepEM2); //Start blocking from em2 for i2c
				i2cTransfer(I2C_READ, read_data, sizeof(read_data));
				next_state = STATE4_I2C_READ_DONE;
			}
			break;

		case STATE4_I2C_READ_DONE:
			/* State to calculate final temp and power off sensor */
			if((event & I2C_TRANSFER_DONE) == I2C_TRANSFER_DONE)
			{
				CORE_ENTER_CRITICAL();
				event &= ~I2C_TRANSFER_DONE;
				CORE_EXIT_CRITICAL();
				sleepBlockEnd(sleepEM2); //Stop blocking at end of i2c transfer
				CalculateTemp(read_data);
				//gpioDisable(TEMP_SENSOR_ENABLE_port, TEMP_SENSOR_ENABLE_pin); //Load power management off
				next_state = STATE0_POWER_UP;
			}
	}

	if(current_state != next_state)
	{
		LOG_INFO("Temp sensor transitioned from state %d to state %d", current_state, next_state);
		current_state = next_state;
	}
	else
	{
		LOG_ERROR("No transition of state. Current State: %d\n", current_state);
	}
}

/*
 * @func - handle_event_BT
 * @brief - State machine for BT
 * @parameters - uint32_t  - event
 * @return - none
 */
void handle_event_BT(uint32_t btEvent)
{
	enum My_States next_state;

	switch(current_state)
	{
		case STATE0_POWER_UP:
			/* State to power up temperature sensor and start 80ms timer */
			if((btEvent & TURN_SI7021_ON) == TURN_SI7021_ON)
			{
				//gpioEn(TEMP_SENSOR_ENABLE_port, TEMP_SENSOR_ENABLE_pin); //Load power management on
				timerSetEventInMS(DELAY_AFTER_SENSOR_POWER_UP_MS); //80ms delay
				next_state = STATE1_I2C_WRITE_START;
			}
			break;

		case STATE1_I2C_WRITE_START:
			/* State to start i2c write to temp sensor */
			if((btEvent & DELAY_DONE) == DELAY_DONE)
			{
				sleepBlockStart(sleepEM2); //Start blocking from em2 for i2c
				write_data[0] = READ_TEMP_NO_HOLD_MASTER;
				i2cTransfer(I2C_WRITE, write_data, sizeof(write_data)); //write 0xF3 (temp)
				next_state = STATE2_I2C_WRITE_DONE;
			}
			break;

		case STATE2_I2C_WRITE_DONE:
			/* State to start 10ms delay between i2c write and read */
			if((btEvent & I2C_TRANSFER_DONE) == I2C_TRANSFER_DONE)
			{
				sleepBlockEnd(sleepEM2); //Stop blocking at end of i2c transfer
				timerSetEventInMS(DELAY_BETWEEN_WRITE_READ_MS); //10ms delay
				next_state = STATE3_I2C_READ_START;
			}
			break;

		case STATE3_I2C_READ_START:
			/* state to start i2c read from temp sensor */
			if((btEvent & DELAY_DONE) == DELAY_DONE)
			{
				sleepBlockStart(sleepEM2); //Start blocking from em2 for i2c
				i2cTransfer(I2C_READ, read_data, sizeof(read_data));
				next_state = STATE4_I2C_READ_DONE;
			}
			break;

		case STATE4_I2C_READ_DONE:
			/* State to calculate final temp and power off sensor */
			if((btEvent & I2C_TRANSFER_DONE) == I2C_TRANSFER_DONE)
			{
				sleepBlockEnd(sleepEM2); //Stop blocking at end of i2c transfer
				CalculateTemp(read_data);
				//gpioDisable(TEMP_SENSOR_ENABLE_port, TEMP_SENSOR_ENABLE_pin); //Load power management off
				next_state = STATE0_POWER_UP;
			}
			break;
	}

	if(current_state != next_state)
	{
		//LOG_INFO("Temp sensor transitioned from state %d to state %d", current_state, next_state);
		current_state = next_state;
	}
	else
	{
		//LOG_INFO("No transition of state. Current State: %d\n", current_state);
	}
}

