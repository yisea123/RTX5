#include "esp8266.h"
#include "esp8266_def.h"
//#include "main.h"
#include "string.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
//#include "fifo.h"
//#include "wifi.h"
//#include "list.h"
//#include "lorawan.h"
#include "debug.h"
//#include "bigendian.h"
#include "uart_server.h"

uint8_t esp8266_rx_buffer[256];

int esp8266_send(const void *pData, size_t size)
{

	return uart2_send(pData, size);
}
int esp8266_recv(const void *pData, size_t timeout)
{
	return uart2_recv(pData, timeout);
}

int esp8266_enterat(const void *pData, size_t size)
{
uint32_t status = 0;
	while(status == 0)
	{
		size_t size = esp8266_recv(esp8266_rx_buffer, 10);
		if(NULL != strstr((char *)esp8266_rx_buffer, (char *)pData))
		{
			
			uart1_send(esp8266_rx_buffer, size);
			uart1_send("\r\n", 2);
			return 0;
		}
	}
	return 1;
}


int esp8266_enterat_1(void)
{
uint32_t status = 0;
	while(status == 0)
	{
		size_t size = esp8266_recv(esp8266_rx_buffer, 10);
		if(NULL != strstr((char *)esp8266_rx_buffer, (char *)READY_TT))
		{
			
			uart1_send(esp8266_rx_buffer, size);
			uart1_send("\r\n", 2);
			return 0;
		}
	}
	return 1;
}

int esp8266_enterat_2(void)
{
uint32_t status = 0;
	while(status == 0)
	{
		size_t size = esp8266_recv(esp8266_rx_buffer, 10);
		if(NULL != strstr((char *)esp8266_rx_buffer, (char *)WIFI_GOT_IP))
		{
			
			uart1_send(esp8266_rx_buffer, size);
			uart1_send("\r\n", 2);
			return 0;
		}
	}
	return 1;
}

int esp8266_setuart(void)
{

	return esp8266_sendat(AT_UART, sizeof(AT_UART));
}

int esp8266_setcwmooe(void)
{

	return esp8266_sendat(AT_CWMOOE, sizeof(AT_CWMOOE));
}
int esp8266_setcwjap(void)
{

	return esp8266_sendat(AT_CWJAP, sizeof(AT_CWJAP));
}
int esp8266_setcipmux(void)
{

	return esp8266_sendat(AT_CIPMUX, sizeof(AT_CIPMUX));
}
int esp8266_setcipstart(void)
{
	esp8266_sendat(AT_CIPSTART, sizeof(AT_CIPSTART));
	return esp8266_enterat("OK", 2);
	
}
int esp8266_setcipmode(void)
{

	return esp8266_sendat(AT_CIPMODE, sizeof(AT_CIPMODE));
}

int esp8266_setcipsend(void)
{

	return esp8266_sendat(AT_CIPSEND, sizeof(AT_CIPSEND));
}

int esp8266_sendat(const void *pData, size_t Size)
{
	int status = esp8266_send(pData, Size);
	if(status != HAL_OK)
	{
		PRINT_ERR("inst esp8266 sendat error \r\n");
		return 1;
	}
	uint32_t size = 0;
	while(size == 0)
	{
		size = esp8266_recv(esp8266_rx_buffer, 10);
		if(size != 0)
		{
			uart1_send(esp8266_rx_buffer, size);
			uart1_send("\r\n", 2);
			return 0;
		}
	}
	return 1;
}


int esp8266_server_init(void)
{
	osDelay(50);

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_1, GPIO_PIN_SET); 
		esp8266_enterat_1();		
	esp8266_setcwmooe();
	esp8266_setuart();
	esp8266_setcipmux();
	esp8266_enterat_2();
	esp8266_setcipstart();
	esp8266_setcipmode();
	esp8266_setcipsend();
	return 0;
	
//	uint8_t READY_TT[] = "ready\r\n";
//	uint8_t AT_UART[] = "AT+UART=115200,8,1,0,0\r\n";
//	uint8_t AT_CWMOOE[] = "AT+CWMODE=1\r\n";
//	uint8_t AT_CWJAP[] = "AT+CWJAP=\"123123\",\"12345678\" \r\n";
//	uint8_t AT_CIPMUX[] = "AT+CIPMUX=0\r\n";
//	uint8_t WIFI_GOT_IP[] = "WIFI GOT IP\r\n";
//	uint8_t AT_CIPSTART[] = "AT+CIPSTART=\"TCP\",\"192.168.137.1\",8001\r\n";
////	uint8_t AT_CIPSTART[] = "AT+CIPSTART=\"TCP\",\"120.76.210.158\",4199\r\n";
//	uint8_t AT_CIPMODE[] = "AT+CIPMODE=1\r\n";
//	uint8_t AT_CIPSEND[] = "AT+CIPSEND\r\n";
//	uint8_t ccctv[30];
////	uint8_t ssize = *a;
//	switch(*a)
//				
//	{
//					case 0:
//						if(NULL != strstr((char *)data, (char *)WIFI_GOT_IP))
//						{
//							*a = 1;	
//							fifo_out(&DATA_FifoRx, (uint8_t *)ccctv, sizeof(WIFI_GOT_IP));
//							WiFi_Date_Transmit((uint8_t *)AT_CIPSTART, sizeof(AT_CIPSTART));
//							
//							WIFI_TX_BUFFER[WIFI_Tx_index].size = sizeof(AT_CIPSTART);
//							memcpy(&WIFI_TX_BUFFER[WIFI_Tx_index].tx_256,(uint8_t *)AT_CIPSTART, WIFI_TX_BUFFER[WIFI_Tx_index].size);
////							WIFI_Tx_index++;
//							ESP_Tx((uint32_t *)&WIFI_TX_BUFFER[WIFI_Tx_index].tx_256, (uint32_t *)&WIFI_TX_BUFFER[WIFI_Tx_index].size);
//						}					
//						break;
//					case 1:
//						if(NULL != strstr((char *)data, "OK"))
//						{
//							
//							*a = 2;
//							fifo_out(&DATA_FifoRx, (uint8_t *)ccctv, 2);
//							osDelay (10);
//							WiFi_Date_Transmit((uint8_t *)AT_CIPMODE, sizeof(AT_CIPMODE));
//							
//							WIFI_TX_BUFFER[WIFI_Tx_index].size = sizeof(AT_CIPMODE);
//							memcpy(&WIFI_TX_BUFFER[WIFI_Tx_index].tx_256,(uint8_t *)AT_CIPMODE, sizeof(AT_CIPMODE));
////							WIFI_Tx_index++;
//							ESP_Tx((uint32_t *)&WIFI_TX_BUFFER[WIFI_Tx_index].tx_256, (uint32_t *)&WIFI_TX_BUFFER[WIFI_Tx_index].size);
//						}
//						break;
//					case 2:
//						if(NULL != strstr((char *)data, "OK"))
//						{
//							*a = 3;
//							fifo_out(&DATA_FifoRx, (uint8_t *)ccctv, 2);
//							WiFi_Date_Transmit((uint8_t *)AT_CIPSEND, sizeof(AT_CIPSEND));
//							
//							WIFI_TX_BUFFER[WIFI_Tx_index].size = sizeof(AT_CIPSEND);
//							memcpy(&WIFI_TX_BUFFER[WIFI_Tx_index].tx_256,(uint8_t *)AT_CIPSEND, sizeof(AT_CIPSEND));
////							WIFI_Tx_index++;
//							ESP_Tx((uint32_t *)&WIFI_TX_BUFFER[WIFI_Tx_index].tx_256, (uint32_t *)&WIFI_TX_BUFFER[WIFI_Tx_index].size);
//						}
//						break;
//					case 3:
//						if(NULL != strstr((char *)data, "OK"))
//						{
//							*a = 4;
//							fifo_out(&DATA_FifoRx, (uint8_t *)ccctv, 2);
//							init = 1;
////							WiFi_ReadyFlag = 0;
//						}
//						break;
//						case 5:
//						if(NULL != strstr((char *)data, (char *)READY_TT))
//						{
//							*a = 6;	
//							fifo_out(&DATA_FifoRx, (uint8_t *)ccctv, sizeof(READY_TT));
//							WiFi_Date_Transmit((uint8_t *)AT_CWMOOE, sizeof(AT_CWMOOE));
//							
//							WIFI_TX_BUFFER[WIFI_Tx_index].size = sizeof(AT_CWMOOE);
//							memcpy(&WIFI_TX_BUFFER[WIFI_Tx_index].tx_256,(uint8_t *)AT_CWMOOE, sizeof(AT_CWMOOE));
////							WIFI_Tx_index++;
//							ESP_Tx((uint32_t *)&WIFI_TX_BUFFER[WIFI_Tx_index].tx_256, (uint32_t *)&WIFI_TX_BUFFER[WIFI_Tx_index].size);
//						}					
//						break;
//						case 6:
//						if(NULL != strstr((char *)data, "OK"))
//						{
//							*a = 7;
//							fifo_out(&DATA_FifoRx, (uint8_t *)ccctv, 2);
//							WiFi_Date_Transmit((uint8_t *)AT_UART, sizeof(AT_UART));
//							
//							WIFI_TX_BUFFER[WIFI_Tx_index].size = sizeof(AT_UART);
//							memcpy(&WIFI_TX_BUFFER[WIFI_Tx_index].tx_256,(uint8_t *)AT_UART, sizeof(AT_UART));
////							WIFI_Tx_index++;
//							ESP_Tx((uint32_t *)&WIFI_TX_BUFFER[WIFI_Tx_index].tx_256, (uint32_t *)&WIFI_TX_BUFFER[WIFI_Tx_index].size);
//						}
//						break;
//						case 7:
//						if(NULL != strstr((char *)data, "OK"))
//						{
//							*a = 0;
//							fifo_out(&DATA_FifoRx, (uint8_t *)ccctv, 2);
//							WiFi_Date_Transmit((uint8_t *)AT_CIPMUX, sizeof(AT_CIPMUX));
//							
//							WIFI_TX_BUFFER[WIFI_Tx_index].size = sizeof(AT_CIPMUX);
//							memcpy(&WIFI_TX_BUFFER[WIFI_Tx_index].tx_256,(uint8_t *)AT_CIPMUX, sizeof(AT_CIPMUX));
////							WIFI_Tx_index++;
//							ESP_Tx((uint32_t *)&WIFI_TX_BUFFER[WIFI_Tx_index].tx_256, (uint32_t *)&WIFI_TX_BUFFER[WIFI_Tx_index].size);
//						}
//						break;
//					
//	}
//	osDelay(50);
////	if(WIFI_Tx_index > 9)
////	{
////		WIFI_Tx_index = 0;
////	}
//				
}

