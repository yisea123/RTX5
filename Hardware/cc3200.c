#include "cc3200.h"
#include "cmsis_os2.h"
#include "debug.h"
#include "cc3200_def.h"
//#include "wifi.h"
#include "uart_server.h"
#include "fifo.h"
#include "string.h"

extern UART_HandleTypeDef huart6;
extern osSemaphoreId_t UART6_Semaphore;
extern osMessageQueueId_t uart6_rx_DMA_MessageQueue_Id;
extern struct kfifo uart6_rxfifo;

uint8_t cc3200_rx_buffer[512];

int cc3200_send(const void *pData, size_t size)
{

	return uart2_send(pData, size);
}

int cc3200_recv(const void *pData, size_t timeout)
{
	return uart2_recv(pData, timeout);
}



int cc3200_enterat_1(void)
{
	
	return cc3200_sendat(AT_Setmode_1, sizeof(AT_Setmode_1));
}

int cc3200_enterat_2(void)
{
	
	return cc3200_sendat(AT_Setmode_2, sizeof(AT_Setmode_2));
}

int cc3200_setwmode(void)
{
	
	return cc3200_sendat(AT_WMODE, sizeof(AT_WMODE));
}

int cc3200_setwsta(void)
{
	
	return cc3200_sendat(AT_WSTA, sizeof(AT_WSTA));
}

int cc3200_setwann(void)
{
	
	return cc3200_sendat(AT_WANN, sizeof(AT_WANN));
}

int cc3200_setwmod(void)
{
	
	return cc3200_sendat(AT_WKMOD, sizeof(AT_WKMOD));
}
int cc3200_setsocka(void)
{
	
	return cc3200_sendat(AT_SOCKA, sizeof(AT_SOCKA));
}
int cc3200_setregena(void)
{
	
	return cc3200_sendat(AT_REGENA, sizeof(AT_REGENA));
}
int cc3200_setregusr(void)
{
	
	return cc3200_sendat(AT_REGUSR, sizeof(AT_REGUSR));
}

int cc3200_setentm(void)
{
	
	return cc3200_sendat(AT_ENTM, sizeof(AT_ENTM));
}
int cc3200_quitat(void)
{
	
	return cc3200_sendat(AT_Z, sizeof(AT_Z));
}

int cc3200_sendat(const void *pData, size_t Size)
{
	int status = cc3200_send(pData, Size - 1);
	if(status != HAL_OK)
	{
		PRINT_ERR("inst register central error \r\n");
		return 1;
	}
	uint32_t size = 0;
	while(size == 0)
	{
		size = cc3200_recv(cc3200_rx_buffer, 10);
		if(size != 0)
		{
			uart1_send(cc3200_rx_buffer, size);
			uart1_send("\r\n", 2);
			return 0;
		}
	}
	return 1;
}

int cc3200_server_init(void)
{
//	text:


		 if(0 != cc3200_enterat_1())
		 {		 
	//		goto text;
			 return 1;
		 }
//		 osDelay(200);
		 if(0 != cc3200_enterat_2())
		 {
	//			goto text;
			 return 1;
		 }
//		 osDelay(200);
		 if(0 != cc3200_setwmode())
		 {
	//			goto text;
			 return 1;
		 }
//		 osDelay(200);
		 if(0 != cc3200_setwsta())
		 {
	//			goto text;
			 return 1;
		 }
		 osDelay(200);
		 if(0 != cc3200_setwann())
		 {
	//			goto text;
			 return 1;
		 }
//		 osDelay(200);
		 if(0 != cc3200_setwmod())
		 {
	//			goto text;
			 return 1;
		 }
//		 osDelay(200);
		 if(0 != cc3200_setsocka())
		 {
	//			goto text;
			 return 1;
		 }
//		 osDelay(200);
		 if(0 != cc3200_setregena())
		 {
	//			goto text;
			 return 1;
		 }
//		 osDelay(200);
		 if(0 != cc3200_setregusr())
		 {
	//			goto text;
			 return 1;
		 }
//			osDelay(200);
		 if(0 != cc3200_quitat())
		 {
				
			 return 1;
		 }
// }
 return 0;
}

void esp_8266(uint8_t *data, uint8_t *a)
{	
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
	switch(*a)
				
	{
					case 0:
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
						break;
					case 1:
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
						break;
					case 2:
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
						break;
					case 3:
//						if(NULL != strstr((char *)data, "OK"))
//						{
//							*a = 4;
//							fifo_out(&DATA_FifoRx, (uint8_t *)ccctv, 2);
//							init = 1;
////							WiFi_ReadyFlag = 0;
//						}
						break;
						case 5:
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
						break;
						case 6:
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
						break;
						case 7:
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
						break;
					
	}
	osDelay(50);
}

//void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
//{
////    if (huart->Instance == USART1)
////        osSemaphoreRelease(UART1_Semaphore);
//    if (huart->Instance == USART6)
//        osSemaphoreRelease(UART6_Semaphore);
//}
