#ifndef __LED_H
#define __LED_H
#include "cmsis_os2.h"
#include "stm32f4xx_hal.h"

#define red_on() 		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET);
#define red_off() 	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_SET);

#define green_on() 	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);
#define green_off() HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);

#define blue_on()	 	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
#define blue_off() 	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);

#define LED_THREAD_PRIORITY osPriorityNormal 
#define LED_THREAD_FLAG_STOP 0x02u


osThreadId_t LED_Init(void);
osThreadId_t LED_DeInit(void);
#endif

