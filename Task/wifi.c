//#include "wifi.h"
//#include "main.h"
//#include "string.h"
//#include "stm32f4xx_hal.h"
//#include "cmsis_os2.h"
//#include "esp8266.h"
//#include "lorawan.h"
//#include "fifo.h"
//#include "debug.h"

//#ifndef WIFI_DBG
//#define PRINT_WIFI_DEG(fmt,arg...)            printf(fmt, ##arg)
//#else
//#define PRINT_WIFI_DEG(fmt,arg...)            {}
//#endif

//extern uint8_t WiFi_ReadyFlag;

//extern int WIFI_Tx_index;
//extern TRANSMIT_BUFFER_SIZE WIFI_TX_BUFFER[];

//osThreadId_t WIFI_ThreadId = NULL;
//osThreadAttr_t WIFI_ThreadAttr = {.priority = osPriorityBelowNormal, .stack_size = 1024};
////osThreadId_t WIFI_Init_ThreadId = NULL;
//osThreadAttr_t WIFI_Init_ThreadAttr = {.priority = osPriorityBelowNormal1, .stack_size = 1024};
////osSemaphoreId_t ESP_Semaphore_Rx;

//void WIFI_Thread(void *arg);
////void WIFI_Init_Thread(void *arg);

//struct kfifo WIFI_FifoTx, WIFI_FifoRx;
//struct kfifo DATA_FifoRx;
//uint8_t WIFI_FifoTxBuffer[WIFI_FIFO_TX_SIZE];
//uint8_t WIFI_FifoRxBuffer[WIFI_FIFO_RX_SIZE];
//uint8_t DATA_FifoRxBuffer[DATA_FIFO_RX_SIZE];
//uint8_t WIFI_TxBuffer[BUFFER_SIZE];
//uint8_t WIFI_RxBuffer[BUFFER_SIZE];
//uint8_t WIFI_InitBuffer[BUFFER_SIZE];
////uint8_t WIFI_esp8266[30];

//extern UART_HandleTypeDef huart1;
//extern UART_HandleTypeDef huart2;
//extern UART_HandleTypeDef huart6;



////#define BUFFER_SIZE 100
///* 串口接收缓冲区 */
//uint8_t usart1_rx_buffer[BUFFER_SIZE] = {0};
//uint8_t usart2_rx_buffer[BUFFER_SIZE] = {0};
//uint8_t usart6_rx_buffer[BUFFER_SIZE] = {0};
///* 串口接收完成标记 */
//uint8_t recv_end_flag1 = 0;
//uint8_t recv_end_flag2 = 0;
//uint8_t recv_end_flag6 = 0;
///* 串口接收数据长度 */
//uint16_t usart1_rx_len = 0;
//uint16_t usart2_rx_len = 0;
//uint16_t usart6_rx_len = 0;

//osThreadId_t WIFI_Init(void)
//{
////    if (fifo_init(&WIFI_FifoTx, WIFI_FifoTxBuffer, WIFI_FIFO_TX_SIZE))
////    {
////        PRINT_ERR("WIFI Tx fifo init failed \r\n");
////        return NULL;
////    }
//    if (fifo_init(&WIFI_FifoRx, WIFI_FifoRxBuffer, WIFI_FIFO_RX_SIZE))
//    {
////        fifo_free(&WIFI_FifoTx);
//        PRINT_ERR("WIFI Rx fifo init failed \r\n");
//        return NULL;
//    }
//		if (fifo_init(&DATA_FifoRx, DATA_FifoRxBuffer, DATA_FIFO_RX_SIZE))
//    {
//        PRINT_ERR("DATA Rx fifo init failed \r\n");
//        return NULL;
//    }
//    if (ESP_Event_OK != ESP_Init())
//    {
////        fifo_free(&WIFI_FifoTx);
//        fifo_free(&WIFI_FifoRx);
//				fifo_free(&DATA_FifoRx);
////        PRINT_ERR("ESP init failed \r\n");
//        return NULL;
//    }
//		osThreadNew(WIFI_Init_Thread, NULL, &WIFI_Init_ThreadAttr);
//    WIFI_ThreadId = osThreadNew(WIFI_Thread, NULL, &WIFI_ThreadAttr);
//		
//    if (WIFI_ThreadId == NULL)
//    {
////        fifo_free(&WIFI_FifoTx);
//        fifo_free(&WIFI_FifoRx);
//				fifo_free(&DATA_FifoRx);
//        if(ESP_DeInit() != ESP_Event_OK)
//            PRINT_ERR("ESP deinit failed \r\n");
//        PRINT_ERR("WIFI Thread creat failed \r\n");
//        return NULL;
//    }
//    return WIFI_ThreadId;
//}

//WIFI_EventTypeDef __WIFI_DeInit(void)
//{
//    //release the resource like the semaphore or mutex
////    fifo_free(&WIFI_FifoTx);
//    fifo_free(&WIFI_FifoRx);
//		fifo_free(&DATA_FifoRx);
//    if (ESP_Event_OK != ESP_DeInit())
//    {
//        PRINT_ERR("esp8266 deinit failed \r\n");
//    }
//    return WIFI_Event_OK;
//}

////void commend(UART_HandleTypeDef *huart, uint8_t *reflag, uint8_t *data, uint16_t *size)
////{
////				if(*reflag ==1)		
////				{         					
////						WiFi_Date_Receive((uint8_t *)data, (uint8_t)*size);
////						
////            *size = 0;
////            *reflag = 0;
////            HAL_UART_Receive_DMA(huart,data,BUFFER_SIZE);
////        }
////}

//void WIFI_Thread(void *arg)
//{
//    uint32_t tmp = 0, size = 0;
////    osStatus_t res;
////	uint8_t WIFI_GOT_IP[] = "WIFI GOT IP\r\n";
//	int index = 0;
//    for (;;)
//    { 
////				WIFI_AT_Command();
////        //thread code begain						
////				if (fifo_used(&WIFI_FifoTx) >= 1)
////        {
////            size = fifo_out(&WIFI_FifoTx, WIFI_TxBuffer, BUFFER_SIZE);
//////						PRINT_DBG("SSize = %d\r\n",size);
//////            if (ESP_Event_OK != ESP_Tx(WIFI_TxBuffer, &size))
//////                PRINT_WARN("wifi tx failed \r\n");
////        }
//				
////				if(WIFI_Tx_index != index )
////				{
//////          size = 256;  
////					memcpy(WIFI_TxBuffer, &WIFI_TX_BUFFER[index].tx_256, WIFI_TX_BUFFER[index].size);
////					if (ESP_Event_OK != ESP_Tx(WIFI_TxBuffer, (uint32_t *)&WIFI_TX_BUFFER[index].size))						
////						PRINT_WARN("wifi tx failed \r\n");
////					index++;
////					if(index > 9)
////						index = 0;
////				}
//				        
//				if (fifo_used(&WIFI_FifoRx) >= 1)
//        {
//            size = fifo_out(&WIFI_FifoRx, WIFI_RxBuffer, BUFFER_SIZE);
//            if (ESP_Event_OK != ESP_Rx(WIFI_RxBuffer, &size))
//                PRINT_WARN("wifi rx failed \r\n");
//        }

//				if(recv_end_flag1 ==1)		
//				{         
////            HAL_UART_Transmit_DMA(&huart1,rx_buffer,rx_len);
////						HAL_UART_Transmit_DMA(&huart2,usart1_rx_buffer,usart1_rx_len);
//						WiFi_Date_Transmit((uint8_t *)&usart1_rx_buffer, (uint8_t)usart1_rx_len);	
//            usart1_rx_len=0;
//            recv_end_flag1=0;
////            HAL_UART_Receive_DMA(&huart1,usart1_rx_buffer,BUFFER_SIZE);
//						WIFI_TX(usart1_rx_buffer);
//        }

//#ifdef esp8266
//				if(recv_end_flag2 ==1)		
//				{         
////            HAL_UART_Transmit_DMA(&huart1,rx_buffer,rx_len);
////						HAL_UART_Transmit_DMA(&huart1,usart2_rx_buffer,usart2_rx_len);
//						WiFi_Date_Receive((uint8_t *)&usart2_rx_buffer, (uint8_t)usart2_rx_len);
////						memcpy(&WIFI_InitBuffer, &usart2_rx_buffer, (uint8_t)usart2_rx_len);
//            usart2_rx_len=0;
//            recv_end_flag2=0;
////            HAL_UART_Receive_DMA(&huart2,usart2_rx_buffer,BUFFER_SIZE);
//					WIFI_RX(usart2_rx_buffer);
//        }
//#else
//				if(recv_end_flag6 ==1)		
//				{         
////            HAL_UART_Transmit_DMA(&huart1,rx_buffer,rx_len);
////						HAL_UART_Transmit_DMA(&huart1,usart2_rx_buffer,usart2_rx_len);
//						WiFi_Date_Receive((uint8_t *)&usart6_rx_buffer, (uint8_t)usart6_rx_len);
////						memcpy(&WIFI_InitBuffer, &usart2_rx_buffer, (uint8_t)usart2_rx_len);
//            usart6_rx_len=0;
//            recv_end_flag6=0;
////            HAL_UART_Receive_DMA(&huart2,usart2_rx_buffer,BUFFER_SIZE);
//					WIFI_RX(usart6_rx_buffer);
//        }
//#endif
//				
////				commend(&huart1, &recv_end_flag1, (uint8_t *)&usart1_rx_buffer, &usart1_rx_len);
////				commend(&huart2, &recv_end_flag2, (uint8_t *)&usart2_rx_buffer, &usart2_rx_len);
//        //thread code end
//        tmp = osThreadFlagsWait(WIFI_THREAD_FLAG_STOP, osFlagsWaitAny, 0);
//        if (!(tmp & 0x80000000u))
//        {
//            if (tmp & WIFI_THREAD_FLAG_STOP)
//            {
//                __WIFI_DeInit();
//            }
//        }
//    }	
//}


//void USART1_IRQHandler(void)
//{
//  /* USER CODE BEGIN USART1_IRQn 0 */
//    uint32_t tmp_flag = 0;
//    uint32_t temp;

//  /* USER CODE END USART1_IRQn 0 */
//  HAL_UART_IRQHandler(&huart1);
//  /* USER CODE BEGIN USART1_IRQn 1 */
//    tmp_flag =  __HAL_UART_GET_FLAG(&huart1,UART_FLAG_IDLE);
//    if((tmp_flag != RESET))
//			{
//            
//        __HAL_UART_CLEAR_IDLEFLAG(&huart1);
//        
//        /* 读取串口状态寄存器（芯片型号不同，寄存器名称可能需要修改） */
//        temp = huart1.Instance->SR;
//        /* 读取串口数据寄存器（芯片型号不同，寄存器名称可能需要修改） */
//        temp = huart1.Instance->DR;
////        HAL_UART_DMAStop(&huart1);
//				HAL_UART_AbortReceive(&huart1);				
//        /* 读取DMA剩余传输数量（芯片型号不同，寄存器名称可能需要修改） */
//        temp  = hdma_usart1_rx.Instance->NDTR;
//        usart1_rx_len =  BUFFER_SIZE - temp;
//        recv_end_flag1 = 1;
//			}
//  /* USER CODE END USART1_IRQn 1 */
//}

//void USART2_IRQHandler(void)
//{
//  /* USER CODE BEGIN USART2_IRQn 0 */
//    uint32_t tmp_flag = 0;
//    uint32_t temp;

//  /* USER CODE END USART2_IRQn 0 */
//  HAL_UART_IRQHandler(&huart2);
//  /* USER CODE BEGIN USART2_IRQn 1 */
//    tmp_flag =  __HAL_UART_GET_FLAG(&huart2,UART_FLAG_IDLE);
//    if((tmp_flag != RESET))
//			{
//            
//        __HAL_UART_CLEAR_IDLEFLAG(&huart2);
//        
//        /* 读取串口状态寄存器（芯片型号不同，寄存器名称可能需要修改） */
//        temp = huart2.Instance->SR;
//        /* 读取串口数据寄存器（芯片型号不同，寄存器名称可能需要修改） */
//        temp = huart2.Instance->DR;
////        HAL_UART_DMAStop(&huart2);
//				HAL_UART_AbortReceive(&huart2);				
//        /* 读取DMA剩余传输数量（芯片型号不同，寄存器名称可能需要修改） */
//        temp  = hdma_usart2_rx.Instance->NDTR;
//        usart2_rx_len =  BUFFER_SIZE - temp;
//        recv_end_flag2 = 1;
//			}
//  /* USER CODE END USART2_IRQn 1 */
//}

//void USART6_IRQHandler(void)
//{
//  /* USER CODE BEGIN USART6_IRQn 0 */
//    uint32_t tmp_flag = 0;
//    uint32_t temp;
//	
//  /* USER CODE END USART6_IRQn 0 */
//  HAL_UART_IRQHandler(&huart6);
//  /* USER CODE BEGIN USART6_IRQn 1 */
//    tmp_flag =  __HAL_UART_GET_FLAG(&huart6,UART_FLAG_IDLE);
//    if((tmp_flag != RESET))
//			{
//            
//        __HAL_UART_CLEAR_IDLEFLAG(&huart6);
//        
//        /* 读取串口状态寄存器（芯片型号不同，寄存器名称可能需要修改） */
//        temp = huart6.Instance->SR;
//        /* 读取串口数据寄存器（芯片型号不同，寄存器名称可能需要修改） */
//        temp = huart6.Instance->DR;
////        HAL_UART_DMAStop(&huart2);
//				HAL_UART_AbortReceive(&huart6);				
//        /* 读取DMA剩余传输数量（芯片型号不同，寄存器名称可能需要修改） */
//        temp  = hdma_usart6_rx.Instance->NDTR;
//        usart6_rx_len =  BUFFER_SIZE - temp;
//        recv_end_flag6 = 1;
//			}
//  /* USER CODE END USART6_IRQn 1 */
//}

