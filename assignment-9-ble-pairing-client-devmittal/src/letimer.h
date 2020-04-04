/*****************************************************************************
​ ​* ​ ​ @file​ ​  		letimer.h
​ * ​ ​ @brief​ ​ 		header file for letimer.c
​ * ​ ​ @Author(s)​  	​​Devansh Mittal
​ * ​ ​ @Date​ ​​ 		September 11th, 2019
​ * ​ ​ @version​ ​ 		1.0
*****************************************************************************/

#ifndef SRC_LETIMER_H_
#define SRC_LETIMER_H_

#include "sleep.h"

volatile int comp1_int;

void timerSetEventInMS(uint32_t ms_until_wakeup);
void timerWaitUs(uint32_t us_wait);
void letimerInit(int comp0, int comp1, SLEEP_EnergyMode_t sleep_mode_blocked);
void sleepBlockStart(SLEEP_EnergyMode_t sleep_mode_blocked);
void sleepBlockEnd(SLEEP_EnergyMode_t sleep_mode_blocked);

#endif /* SRC_LETIMER_H_ */
