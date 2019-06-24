#ifndef __M8321_H
#define __M8321_H
#include "stm32f4xx_hal.h"

#define pwrkey_on()  		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET)
#define pwrkey_off()		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET)

#define resetin_on()  	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET)
#define resetin_off()		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET)

#define wakeupin_on()  	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET)
#define wakeupin_off()	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET)

#define wakeupout()			HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10)
#endif