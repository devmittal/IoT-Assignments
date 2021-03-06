/*
 * gpio.c
 *
 *  Created on: Dec 12, 2018
 *      Author: Dan Walkes
 */
#include "gpio.h"
#include <string.h>
#include "event.h"
#include "native_gecko.h"

/**
 * TODO: define these.  See the radio board user guide at https://www.silabs.com/documents/login/user-guides/ug279-brd4104a-user-guide.pdf
 * and GPIO documentation at https://siliconlabs.github.io/Gecko_SDK_Doc/efm32g/html/group__GPIO.html
 */
#define	LED0_port gpioPortF
#define LED0_pin  4
#define LED1_port gpioPortF
#define LED1_pin  5

void gpioInit()
{
	//GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthStrongAlternateStrong);
	GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthWeakAlternateWeak);
	GPIO_PinModeSet(LED0_port, LED0_pin, gpioModePushPull, false);
	//GPIO_DriveStrengthSet(LED1_port, gpioDriveStrengthStrongAlternateStrong);
	GPIO_DriveStrengthSet(LED1_port, gpioDriveStrengthWeakAlternateWeak);
	GPIO_PinModeSet(LED1_port, LED1_pin, gpioModePushPull, false);

	/* Configure PB0 as input and enable interrupt */
	GPIO_PinModeSet(PB0_port, PB0_pin, gpioModeInput, 1);
	GPIO_ExtIntConfig(PB0_port, PB0_pin, 4, true, true, true);
	NVIC_EnableIRQ(GPIO_EVEN_IRQn);
}

/*
 * @func - GPIO_EVEN_IRQHandler
 * @brief - Indicate button press and clear interrupts
 * @parameters - none
 * @return - none
 */
void GPIO_EVEN_IRQHandler(void)
{
	uint32_t int_flag = GPIO_IntGet();

	GPIO_IntClear(int_flag);

	gecko_external_signal(BUTTON_PRESSED);
}

void gpioLed0SetOn()
{
	GPIO_PinOutSet(LED0_port,LED0_pin);
}
void gpioLed0SetOff()
{
	GPIO_PinOutClear(LED0_port,LED0_pin);
}
void gpioLed1SetOn()
{
	GPIO_PinOutSet(LED1_port,LED1_pin);
}
void gpioLed1SetOff()
{
	GPIO_PinOutClear(LED1_port,LED1_pin);
}

void gpioLED0Toggle()
{
	GPIO_PinOutToggle(LED0_port,LED0_pin);
}

void gpioLED1Toggle()
{
	GPIO_PinOutToggle(LED1_port,LED1_pin);
}

void gpioTempSensorEn()
{
	GPIO_PinOutSet(SENSOR_ENABLE_port,SENSOR_ENABLE_pin);
}

void gpioTempSensorDisable()
{
	GPIO_PinOutClear(SENSOR_ENABLE_port,SENSOR_ENABLE_pin);
}

void gpioEn(GPIO_Port_TypeDef port, unsigned int pin)
{
	GPIO_PinOutSet(port, pin);
}

void gpioDisable(GPIO_Port_TypeDef port, unsigned int pin)
{
	GPIO_PinOutClear(port, pin);
}

void gpioEnableDisplay()
{
	GPIO_PinOutSet(SENSOR_ENABLE_port, SENSOR_ENABLE_pin);
}

void gpioSetDisplayExtcomin(bool high)
{
	high ? GPIO_PinOutSet(EXTCOMIN_port, EXTCOMIN_pin) : GPIO_PinOutClear(EXTCOMIN_port, EXTCOMIN_pin);
}
