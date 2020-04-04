/*
 * gpio.h
 *
 *  Created on: Dec 12, 2018
 *      Author: Dan Walkes
 */

#ifndef SRC_GPIO_H_
#define SRC_GPIO_H_
#include <stdbool.h>
#include "em_gpio.h"

#define SENSOR_ENABLE_port gpioPortD
#define SENSOR_ENABLE_pin 	15
#define EXTCOMIN_port 	   gpioPortD
#define EXTCOMIN_pin	    13
#define PB0_port		   gpioPortF
#define PB0_pin				6

#define GPIO_SET_DISPLAY_EXT_COMIN_IMPLEMENTED 	1
#define GPIO_DISPLAY_SUPPORT_IMPLEMENTED		1

void gpioInit();
void gpioLed0SetOn();
void gpioLed0SetOff();
void gpioLed1SetOn();
void gpioLed1SetOff();
void gpioLED0Toggle();
void gpioEn(GPIO_Port_TypeDef port, unsigned int pin);
void gpioDisable(GPIO_Port_TypeDef port, unsigned int pin);
void gpioEnableDisplay();
void gpioSetDisplayExtcomin(bool high);

#endif /* SRC_GPIO_H_ */
