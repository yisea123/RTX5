#include "led.h"
#include "cmsis_os2.h"
#include "debug.h"

//extern uint8_t WiFi_ReadyFlag;
	
osThreadId_t LED_Thread_Id;
const osThreadAttr_t LED_Thread_Attr = {.priority = LED_THREAD_PRIORITY};
void LED_Thread(void *argument);

osThreadId_t LED_Init(void)
{
	LED_Thread_Id = osThreadNew(LED_Thread, NULL, &LED_Thread_Attr);
	if (LED_Thread_Id == NULL)
	{
		PRINT_ERR("LED Thread create failed \r\n");
		return NULL;
	}
	return LED_Thread_Id;
}

void LED_Thread(void *argument)
{
	uint32_t tmp = 0;
	for(;;)
	{
//		if(WiFi_ReadyFlag)
//		{
//			blue_on();
//			osDelay(50);
//			blue_off();
//			osDelay(50);
//		}
//		else
		{
			green_on();
			osDelay(500);
			green_off();
			osDelay(500);
		}
	
		tmp = osThreadFlagsWait(LED_THREAD_FLAG_STOP, osFlagsWaitAny, 0);
		if (!(tmp & 0x80000000u))
		{
			if (tmp & LED_THREAD_FLAG_STOP)
			{
//				__LED_DeInit();
			}
		}
	}
}

osThreadId_t LED_DeInit(void)
{
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0|GPIO_PIN_13, GPIO_PIN_SET);
}