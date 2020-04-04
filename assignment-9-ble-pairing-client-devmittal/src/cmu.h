/*****************************************************************************
​ ​* ​ ​ @file​ ​  		cmu.h
​ * ​ ​ @brief​ ​ 		header file for cmu.c
​ * ​ ​ @Author(s)​  	​​Devansh Mittal
​ * ​ ​ @Date​ ​​ 		September 11th, 2019
​ * ​ ​ @version​ ​ 		1.0
*****************************************************************************/

#ifndef SRC_CMU_H_
#define SRC_CMU_H_

#include "sleep.h"

int clockInit(SLEEP_EnergyMode_t sleep_mode_blocked, float led_period, float freq);

#endif /* SRC_CMU_H_ */
