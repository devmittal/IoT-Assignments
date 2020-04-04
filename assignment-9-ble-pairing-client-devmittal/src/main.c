/*****************************************************************************
​ ​* ​ ​ @file​ ​  		main.c
​ * ​ ​ @brief​ ​ 		Initializes clock and letimer and sleeps in an infinite loop
					Note: Current build disables logging. Screenshots and answers
					to questions match this build. Change to logging build to see
					log outputs.
​ * ​ ​ @Author(s)​  	​​Devansh Mittal
​ * ​ ​ @Date​ ​​ 		September 11th, 2019
​ * ​ ​ @version​ ​ 		1.0
*****************************************************************************/

/* Board headers */
#include "init_mcu.h"
#include "init_board.h"
#include "init_app.h"
#include "ble-configuration.h"
#include "board_features.h"

/* Bluetooth stack headers */
#include "bg_types.h"
#include "native_gecko.h"
#include "gatt_db.h"

/* Libraries containing default Gecko configuration values */
#include "em_emu.h"
#include "sleep.h"
#include "em_core.h"

/* Device initialization header */
#include "hal-config.h"

#if defined(HAL_CONFIG)
#include "bsphalconfig.h"
#else
#include "bspconfig.h"
#endif

#include "em_device.h"
#include "em_chip.h"
#include "em_letimer.h"

/* User define files */
#include "gpio.h"
#include "cmu.h"
#include "letimer.h"
#include "main.h"
#include "log.h"
#include "temp.h"
#include "event.h"
#include "src/myGecko.h"
#include "display.h"
#include "ble_device_type.h"

#ifndef MAX_CONNECTIONS
#define MAX_CONNECTIONS 4
#endif

uint8_t bluetooth_stack_heap[DEFAULT_BLUETOOTH_HEAP(MAX_CONNECTIONS)];

// Gecko configuration parameters (see gecko_configuration.h)
static const gecko_configuration_t config = {
  .config_flags = 0,
  .sleep.flags = SLEEP_FLAGS_DEEP_SLEEP_ENABLE,
  .bluetooth.max_connections = MAX_CONNECTIONS,
  .bluetooth.heap = bluetooth_stack_heap,
  .bluetooth.heap_size = sizeof(bluetooth_stack_heap),
  .bluetooth.sleep_clock_accuracy = 100, // ppm
  .gattdb = &bg_gattdb_data,
  .ota.flags = 0,
  .ota.device_name_len = 3,
  .ota.device_name_ptr = "OTA",
#if (HAL_PA_ENABLE) && defined(FEATURE_PA_HIGH_POWER)
  .pa.config_enable = 1, // Enable high power PA
  .pa.input = GECKO_RADIO_PA_INPUT_VBAT, // Configure PA input to VBAT
#endif // (HAL_PA_ENABLE) && defined(FEATURE_PA_HIGH_POWER)
};

int main(void)
{
  int comp0, comp1;
  float led_period;

  /* Variable defining the blocked sleep mode
   * Eg: sleepEM3 indicates that the system should be in EM2; EM3 and EM4 is blocked.
   * Note: Ensure to cycle power (move switch to BAT and back to AEM) after uploading code when changing energy modes*/
  const SLEEP_EnergyMode_t sleep_mode_blocked=sleepEM4;

  struct gecko_cmd_packet* evt;

  // Initialize device
  initMcu();

  // Initialize board
  initBoard();

  // Initialize application
  initApp();

  // Initialize logging
  logInit();
  initApp();

  LOG_INFO("IoT Assignment 4\n");

  eventInit();

  // Initialize GPIO
  gpioInit();

  // Initialize temperature sensor I2C
  i2cInit();

  //Initialize variables/enums used in events
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

  // Initialize BLE stack.
  gecko_init(&config);

  // Initialize display
  displayInit();

  /* Infinite loop */
  while (1) {
	  	/* Check for stack event. */
	  evt = gecko_wait_event();

#if (BUILD_INCLUDES_BLE_CLIENT)
	  gecko_ecen5823_update_client(evt);
#else
	  gecko_ecen5823_update(evt);
#endif
  }
}
