/*****************************************************************************
​ ​* ​ ​ @file​ ​  		cmu.c
​ * ​ ​ @brief​ ​ 		source file to initialize clocks
​ * ​ ​ @Author(s)​  	​​Devansh Mittal
​ * ​ ​ @Date​ ​​ 		September 11th, 2019
​ * ​ ​ @version​ ​ 		1.0
*****************************************************************************/

#include "cmu.h"
#include "em_cmu.h"
#include <math.h>
#include "main.h"

/*
 * @func - clockInit
 * @brief - Initialize clocks
 * @parameters - SLEEP_EnergyMode_t - sleep mode (and lower) to be blocked
 * 				 float - Period of LED blink
 * 				 float - Frequency of oscillator
 * @return - prescalar
 */
int clockInit(SLEEP_EnergyMode_t sleep_mode_blocked, float led_period, float freq)
{
	CMU_Osc_TypeDef osc;
	CMU_Select_TypeDef ref;
	float pre_div;
	int log_div, div;

	if(sleep_mode_blocked == sleepEM4)
	{
		osc = cmuOsc_ULFRCO;
		ref = cmuSelect_ULFRCO;
	}
	else
	{
		osc = cmuOsc_LFXO;
		ref = cmuSelect_LFXO;
	}

	/* Calculate prescalar value based on freq and blink period */
	pre_div = (((led_period * (freq/COUNT))/1000));

	if(pre_div < 1)
		div = 1;
	else
	{
		log_div = ceil(log(pre_div)/log(2));
		div = pow(2,log_div);
	}

	CMU_OscillatorEnable(osc, true, true);
	CMU_ClockSelectSet(cmuClock_LFA, ref);
	CMU_ClockEnable(cmuClock_LFA, true);
	CMU_ClockDivSet(cmuClock_LETIMER0, div);
	CMU_ClockEnable(cmuClock_LETIMER0, true);

	return div;
}


