/*****************************************************************************
​ ​* ​ ​ @file​ ​  		main.h
​ * ​ ​ @brief​ ​ 		contains misc macros needed by multiple source codes
​ * ​ ​ @Author(s)​  	​​Devansh Mittal
​ * ​ ​ @Date​ ​​ 		September 11th, 2019
​ * ​ ​ @version​ ​ 		1.0
*****************************************************************************/


#ifndef SRC_MAIN_H_
#define SRC_MAIN_H_

#include "native_gecko.h"

#define COUNT 65535
#define LXFO_FREQ 32768
#define ULFRCO_FREQ 1000
#define DELAY_AFTER_SENSOR_POWER_UP_MS 80
#define DELAY_BETWEEN_WRITE_READ_MS 10

/* Define period and On time in ms here respectively */
#define PERIOD 1000
#define ON_TIME 175

#define I2C_READ 1
#define I2C_WRITE 0

/* Formula to compute comp0 and comp1 values */
#define LETIMER_VALUE_FROM_TIME(led_time, frequency, prescalar) \
	((led_time) * (frequency))/((prescalar) * (1000))

/* Formula to compute ms from ticks */
#define TIME_FROM_LETIMER_TICKS(timer_ticks, frequency, clk_prescalar) \
	((timer_ticks) * (1000) * (clk_prescalar))/(frequency)

void handle_gecko_event(uint32_t evt_id, struct gecko_cmd_packet *evt);

/* Global frequency and prescalar value visible to all files which need it */
float freq;
int clk_div;

#endif /* SRC_MAIN_H_ */
