#ifndef __SX1280_H
#define __SX1280_H
#include "stm32f4xx_hal.h"

#define SIGNAL_FLAG_LORA_IRQ	0x0001

typedef enum
{
	LOR_EVENT_OK,
	LOR_EVENT_ERR,
	LOR_EVENT_TIMEOUT,
	LOR_EVENT_NULL
}LOR_Event;


void LOR_SetCmd(uint32_t size);
LOR_Event LOR_Init(void);
LOR_Event LOR_DeInit(void);
LOR_Event LOR_Receive(uint8_t* data, uint32_t* size, uint16_t timeout);
LOR_Event LOR_Transmit(uint8_t* data, uint32_t size, uint16_t timeout);
void LOR_EXTI_Callback(void);

#endif
