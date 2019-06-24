#ifndef __DW1000_H
#define __DW1000_H

#include "cmsis_os2.h"
#define DECAWAVE_CHANNEL DEC_CHANNEL_1

typedef enum
{
	DEC_EVENT_OK = 1,
	DEC_EVENT_ERR = 2,
	DEC_EVENT_TIMEOUT = 3,
} DEC_Event;



DEC_Event DEC_Init(void);
DEC_Event DEC_Receive(uint8_t *timeStamp);
DEC_Event DEC_Transmit(uint8_t *timeStamp);
DEC_Event DEC_GetRxTime(uint8_t *timeStamp);
DEC_Event DEC_GetTxTime(uint8_t *timeStamp);
DEC_Event DEC_AcquireAnchorMessage(uint8_t *data, uint32_t len);;
DEC_Event DEC_DecaTimeAdd(uint8_t *start, uint32_t interval, uint8_t *stop);
uint32_t DEC_DecaTimeToUint32_t(uint8_t *timeStamp);
DEC_Event DEC_GetSystemTime(uint8_t *currentTime);
void DEC_EXTI_Callback(void);
DEC_Event DEC_TRXCancel(void);
DEC_Event DEC_Sleep(void);
DEC_Event DEC_DeInit(void);
DEC_Event DEC_Wakeup(void);
DEC_Event DEC_GetSignalPower(uint16_t signal_power);
#endif
