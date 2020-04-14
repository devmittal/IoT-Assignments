#include <stdio.h>
#include <stdbool.h>
#include "native_gecko.h"
#include "log.h"
#include "letimer.h"
#include "main.h"
#include "cmu.h"
#include "display.h"
#include "myGecko.h"
#include "gpio.h"

extern void gecko_main_init();
bool mesh_bgapi_listener(struct gecko_cmd_packet *evt);

int main(void)
{
	int comp0, comp1;
	float led_period;

	/* Variable defining the blocked sleep mode
	* Eg: sleepEM3 indicates that the system should be in EM2; EM3 and EM4 is blocked.
	* Note: Ensure to cycle power (move switch to BAT and back to AEM) after uploading code when changing energy modes*/
	const SLEEP_EnergyMode_t sleep_mode_blocked=sleepEM4;

	// Initialize stack
	gecko_main_init();

	logInit();

	// Initialize GPIO
	gpioInit();

	led_period = PERIOD;

	if(sleep_mode_blocked == sleepEM4)
	  freq = ULFRCO_FREQ;
	else
	  freq = LXFO_FREQ;

	// Initialize clocks
	clk_div = clockInit(sleep_mode_blocked, led_period, freq);

	comp0 = LETIMER_VALUE_FROM_TIME(PERIOD, freq, clk_div);
	comp1 = comp0 - LETIMER_VALUE_FROM_TIME(ON_TIME, freq, clk_div);

	// Initialize Letimer
	letimerInit(comp0, comp1, sleep_mode_blocked);

	displayInit();

	LOG_INFO("Mesh Assignment");

	/* Infinite loop */
	while (1)
	{
		struct gecko_cmd_packet *evt = gecko_wait_event();
		bool pass = mesh_bgapi_listener(evt);
		if (pass)
		{
			gecko_ecen5823_update(BGLIB_MSG_ID(evt->header), evt);
		}
	}
}
