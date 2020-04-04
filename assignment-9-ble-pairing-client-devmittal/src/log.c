/*
 * log.c
 *
 *  Created on: Dec 18, 2018
 *      Author: Dan Walkes
 */

#include "retargetserial.h"
#include "log.h"
#include "main.h"
#include "em_letimer.h"
#include <stdbool.h>

/**
 * @return a timestamp value for the logger, typically based on a free running timer.
 * This will be printed at the beginning of each log message.
 */
uint32_t loggerGetTimestamp(void)
{
	uint32_t ticks;

	ticks = LETIMER_CompareGet(LETIMER0, 0) - LETIMER_CounterGet(LETIMER0);

	return (TimeStamp_count * PERIOD) + TIME_FROM_LETIMER_TICKS(ticks, freq, clk_div);
}

#if INCLUDE_LOGGING
void logFlush()
{
	RETARGET_SerialFlush();
}
#endif

/**
 * Initialize logging for Blue Gecko.
 * See https://www.silabs.com/community/wireless/bluetooth/forum.topic.html/how_to_do_uart_loggi-ByI
 */
#if INCLUDE_LOGGING
void logInit(void)
{
	RETARGET_SerialInit();
	/**
	 * See https://siliconlabs.github.io/Gecko_SDK_Doc/efm32g/html/group__RetargetIo.html#ga9e36c68713259dd181ef349430ba0096
	 * RETARGET_SerialCrLf() ensures each linefeed also includes carriage return.  Without it, the first character is shifted in TeraTerm
	 */
	RETARGET_SerialCrLf(true);
	LOG_INFO("Initialized Logging");
}
#endif
