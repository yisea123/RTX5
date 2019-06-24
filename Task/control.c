#include "control.h"
#include "cmsis_os2.h"
#include "string.h"
//#include "wifi.h"
//#include "tfcard.h"
//#include "bq24210.h"
//#include "bq27542.h"
//#include "sx1280.h"
//#include "config.h"
#include "debug.h"
//#include "lorawan.h"
//#include "esp8266.h"
#include "led.h"
#include "uart_server.h"
//#include "trans.h"
#include "m8321.h"
#include "dw1000.h"

osThreadId_t CTL_Thread_Id;
osThreadAttr_t CTL_Thread_Attr = {.priority = CTL_THREAD_PRIORITY, .stack_size = 512};
//extern uint8_t WiFi_ReadyFlag;

void CTL_Thread(void *arg);


osThreadId_t CTL_Init(void)
{
	CTL_Thread_Id = osThreadNew(CTL_Thread, NULL, &CTL_Thread_Attr);
	if (CTL_Thread_Id == NULL)
	{
		PRINT_ERR("CTL Thread create failed \r\n");
		return NULL;
	}
	PRINT_DBG("Init ...");
	return CTL_Thread_Id;
}

void CTL_Thread(void *arg)
{
	if(NULL != UART_SERVER_Init())
	{

	}
//	TRANS_Init();		
//	BQ24210_Init();
//	LOW_Init();
	LED_Init();
//	resetin_on();
//	osDelay(6000);
//	resetin_off();
//	
//	pwrkey_on();
//	osDelay(6000);
//	pwrkey_off();
//	
//	wakeupin_on();
//	osDelay(500);
	PRINT_DBG("start\r\n");
	DEC_Init();
	for (;;)
	{
//		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
//		osDelay(500);
//		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
		PRINT_DBG("wake up out : %d\r\n", wakeupout());
		osDelay(500);		
	}
}
