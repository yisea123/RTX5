#ifndef __CONTROL_H
#define __CONTROL_H
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"

extern osMutexId_t spi2_mutex;
extern osMutexId_t debug_mutex;
extern osSemaphoreId_t debug_semaphore;
extern osThreadId_t CTL_Thread_Id;
#define CTL_THREAD_PRIORITY osPriorityRealtime
osThreadId_t CTL_Init(void);
void CTL_DeInit(void);
#endif
