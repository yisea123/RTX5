#include "uart_server.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
#include "debug.h"
#include "fifo.h"
#include "bigendian.h"
#include "cc3200.h"


extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart6;

osThreadId_t UART_SERVER_Thread_Id;
osThreadAttr_t UART_SERVER_Thread_Attr = {.priority = osPriorityNormal, .stack_size = 512};

osSemaphoreId_t UART1_Semaphore;
osSemaphoreAttr_t UART1_Semaphore_Attr = {.name = "UART1_Semaphore"};

osSemaphoreId_t UART2_Semaphore;
osSemaphoreAttr_t UART2_Semaphore_Attr = {.name = "UART2_Semaphore"};

osSemaphoreId_t UART6_Semaphore;
osSemaphoreAttr_t UART6_Semaphore_Attr = {.name = "UART6_Semaphore"};

osMessageQueueAttr_t uart1_rx_DMA_MessageQueue_Attr = {.name = "uart1_rx_DMA_MessageQueue"};
osMessageQueueId_t uart1_rx_DMA_MessageQueue_Id;
#define uart1_rx_DMA_MESSAGE_POOL_SIZE 10
uint32_t uart1_rx_DMA_MessagePool[uart1_rx_DMA_MESSAGE_POOL_SIZE];

osMessageQueueAttr_t uart2_rx_DMA_MessageQueue_Attr = {.name = "uart2_rx_DMA_MessageQueue"};
osMessageQueueId_t uart2_rx_DMA_MessageQueue_Id;
#define uart2_rx_DMA_MESSAGE_POOL_SIZE 10
uint32_t uart2_rx_DMA_MessagePool[uart2_rx_DMA_MESSAGE_POOL_SIZE];

osMessageQueueAttr_t uart6_rx_DMA_MessageQueue_Attr = {.name = "uart6_rx_DMA_MessageQueue"};
osMessageQueueId_t uart6_rx_DMA_MessageQueue_Id;
#define uart6_rx_DMA_MESSAGE_POOL_SIZE 10
uint32_t uart6_rx_DMA_MessagePool[uart6_rx_DMA_MESSAGE_POOL_SIZE];

struct kfifo uart1_rxfifo;
struct kfifo uart2_rxfifo;
struct kfifo uart6_rxfifo;

uint8_t uart1_fifobuffer[1024];
uint8_t uart2_fifobuffer[1024];
uint8_t uart6_fifobuffer[1024];

/* 串口接收缓冲区 */
uint8_t uart1_rx_buffer[BUFFER_SIZE] = {0};
uint8_t uart1_tx_buffer[BUFFER_SIZE] = {0};
uint8_t uart2_rx_buffer[BUFFER_SIZE] = {0};
uint8_t uart2_tx_buffer[BUFFER_SIZE] = {0};
uint8_t uart6_rx_buffer[BUFFER_SIZE] = {0};
uint8_t uart6_tx_buffer[BUFFER_SIZE] = {0};
void UART_SERVER_Thread(void *arg);

osThreadId_t UART_SERVER_Init(void)
{
		if (fifo_init(&uart1_rxfifo, uart1_fifobuffer, 1024))
    {
        PRINT_ERR("uart1 Rx fifo init failed \r\n");
        goto exit;
    }

		if (fifo_init(&uart2_rxfifo, uart2_fifobuffer, 1024))
    {
        PRINT_ERR("uart2 Rx fifo init failed \r\n");
        goto exit;
    }
		
		if (fifo_init(&uart6_rxfifo, uart6_fifobuffer, 1024))
    {
        PRINT_ERR("uart6 Rx fifo init failed \r\n");
        goto exit;
    }

		UART1_Semaphore = osSemaphoreNew(1, 0, &UART1_Semaphore_Attr);    
		if (UART1_Semaphore == NULL)    
		{        
			PRINT_ERR("uart1 semaphore create failed \r\n"); 
			goto exit;
		}

		UART2_Semaphore = osSemaphoreNew(1, 0, &UART2_Semaphore_Attr);    
		if (UART2_Semaphore == NULL)    
		{        
			PRINT_ERR("uart2 semaphore create failed \r\n");
			goto exit;
		}
		
		UART6_Semaphore = osSemaphoreNew(1, 0, &UART6_Semaphore_Attr);    
		if (UART6_Semaphore == NULL)    
		{        
			PRINT_ERR("uart6 semaphore create failed \r\n"); 
			goto exit;
		}
		
		uart1_rx_DMA_MessageQueue_Id = osMessageQueueNew(10, sizeof(uint32_t), &uart1_rx_DMA_MessageQueue_Attr);
    if (uart1_rx_DMA_MessageQueue_Id == NULL)
    {
        PRINT_ERR("uart1 rx message queue create failed \r\n");
        goto exit;
    }

		uart2_rx_DMA_MessageQueue_Id = osMessageQueueNew(10, sizeof(uint32_t), &uart2_rx_DMA_MessageQueue_Attr);
    if (uart2_rx_DMA_MessageQueue_Id == NULL)
    {
        PRINT_ERR("uart2 rx message queue create failed \r\n");
        goto exit;
    }
		
		uart6_rx_DMA_MessageQueue_Id = osMessageQueueNew(10, sizeof(uint32_t), &uart6_rx_DMA_MessageQueue_Attr);
    if (uart6_rx_DMA_MessageQueue_Id == NULL)
    {
        PRINT_ERR("uart6 rx message queue create failed \r\n");
        goto exit;
    }	

    if (0 != uart_server_rx_HardwareInit())
    {
//				fifo_free(&uart1_rxfifo);
//				fifo_free(&uart6_rxfifo);
//				osMessageQueueDelete(uart6_rx_DMA_MessageQueue_Id);
        PRINT_ERR("ESP init failed \r\n");
				goto exit;
//        return NULL;
    }
		
    UART_SERVER_Thread_Id = osThreadNew(UART_SERVER_Thread, NULL, &UART_SERVER_Thread_Attr);
    if (UART_SERVER_Thread_Id == NULL)
    {
        PRINT_ERR("CTL Thread create failed \r\n");
			goto exit;
//        return NULL;
    }
    return UART_SERVER_Thread_Id;
		
exit:
				fifo_free(&uart1_rxfifo);
				fifo_free(&uart2_rxfifo);
				fifo_free(&uart6_rxfifo);
				osMessageQueueDelete(uart1_rx_DMA_MessageQueue_Id);
				osMessageQueueDelete(uart2_rx_DMA_MessageQueue_Id);
				osMessageQueueDelete(uart6_rx_DMA_MessageQueue_Id);	
		return NULL;		
}

int forward1_2(void)
{
		uint32_t count;
		count = uart1_recv(uart2_tx_buffer, 10);
		if (count != 0)
		{

			uart2_send(uart2_tx_buffer, count);
		}
		
		count = uart2_recv(uart1_tx_buffer, 10);
		if (count != 0)
		{

			uart1_send(uart1_tx_buffer, count);	
		}	

	return 0;
}

int forward1_6(void)
{
		uint32_t count;
		count = uart1_recv(uart6_tx_buffer, 10);
		if (count != 0)
		{

			uart6_send(uart6_tx_buffer, count);
		}
		
		count = uart6_recv(uart1_tx_buffer, 10);
		if (count != 0)
		{

			uart1_send(uart1_tx_buffer, count);	
		}	

	return 0;
}

int forward2_6(void)
{
		uint32_t count;
		count = uart2_recv(uart6_tx_buffer, 10);
		if (count != 0)
		{

			uart6_send(uart6_tx_buffer, count);
		}
		
		count = uart6_recv(uart2_tx_buffer, 10);
		if (count != 0)
		{

			uart2_send(uart2_tx_buffer, count);	
		}	

	return 0;
}

void UART_SERVER_Thread(void *arg)
{
//	osStatus_t res;
//	uint32_t uart1_rx_ndtr, uart6_rx_ndtr;

//	cc3200_server_init();
	for(;;)
	{
		forward1_2();
				
		osDelay(2);
	}
	

}



void uart6_receive_dma(void)
{
	HAL_UART_Receive_DMA(&huart6,uart6_rx_buffer,BUFFER_SIZE);
}

int uart_server_rx_HardwareInit(void)// create the ESP_Semaphore_Rx
{

//	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_1, GPIO_PIN_RESET);
//	osDelay (50);
//	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_1, GPIO_PIN_SET);  
	
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);

	HAL_UART_Receive_DMA(&huart1,uart1_rx_buffer,BUFFER_SIZE);

	__HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);

	HAL_UART_Receive_DMA(&huart2,uart2_rx_buffer,BUFFER_SIZE);

	__HAL_UART_ENABLE_IT(&huart6, UART_IT_IDLE);

	HAL_UART_Receive_DMA(&huart6,uart6_rx_buffer,BUFFER_SIZE);

	return 0;
}

int uart1_send(const void *pData, size_t size)
{
	osStatus_t res;
	HAL_UART_Transmit_DMA(&huart1, (uint8_t *)pData, size);
	res = osSemaphoreAcquire(UART1_Semaphore, 100);
	if (res == osOK)
	{
//		PRINT_DBG("uart1 tx success\r\n");
	return 0;
	}
	else
	{
		PRINT_DBG("uart1 tx error\r\n");		
		return 1;
	}
}

int uart1_recv(const void *pData, size_t timeout)
{
	uint32_t uart1_rx_ndtr;
	osStatus_t res = osMessageQueueGet(uart1_rx_DMA_MessageQueue_Id, &uart1_rx_ndtr, NULL, timeout);				 						
	if (res == osOK)       		
	{	
//		PRINT_DBG("uart1 rx size %d\r\n", uart1_rx_ndtr);
		uint32_t size = fifo_out(&uart1_rxfifo, (void *)pData, uart1_rx_ndtr);
		HAL_UART_Receive_DMA(&huart1,uart1_rx_buffer,BUFFER_SIZE);	
//		PRINT_DBG("size = %d\r\n", size);
		if(size != uart1_rx_ndtr)			
		{				
			PRINT_ERR("uart1 rx fifo error\r\n");
			return 0;	
		}
		return size;		
	}
	return 0;
	
}

int uart2_send(const void *pData, size_t size)
{
	osStatus_t res;
	HAL_UART_Transmit_DMA(&huart2, (uint8_t *)pData, size);
	res = osSemaphoreAcquire(UART2_Semaphore, 100);
	if (res == osOK)
	{
//		PRINT_DBG("uart1 tx success\r\n");
	return 0;
	}
	else
	{
		PRINT_DBG("uart2 tx error\r\n");		
		return 1;
	}
}

int uart2_recv(const void *pData, size_t timeout)
{
	uint32_t uart2_rx_ndtr;
	osStatus_t res = osMessageQueueGet(uart2_rx_DMA_MessageQueue_Id, &uart2_rx_ndtr, NULL, timeout);				 						
	if (res == osOK)       		
	{	
//		PRINT_DBG("uart1 rx size %d\r\n", uart1_rx_ndtr);
		uint32_t size = fifo_out(&uart2_rxfifo, (void *)pData, uart2_rx_ndtr);
		HAL_UART_Receive_DMA(&huart2,uart2_rx_buffer,BUFFER_SIZE);	
//		PRINT_DBG("size = %d\r\n", size);
		if(size != uart2_rx_ndtr)			
		{				
			PRINT_ERR("uart2 rx fifo error\r\n");
			return 0;	
		}
		return size;		
	}
	return 0;
}

int uart6_send(const void *pData, size_t size)
{
	osStatus_t res;
	HAL_UART_Transmit_DMA(&huart6, (uint8_t *)pData, size);
	res = osSemaphoreAcquire(UART6_Semaphore, 100);
	if (res == osOK)
	{
//		PRINT_DBG("uart6 tx success\r\n");
	return 0;
	}
	else
	{
		PRINT_DBG("uart6 tx error\r\n");		
		return 1;
	}
}

int uart6_recv(const void *pData, size_t timeout)
{
	uint32_t uart6_rx_ndtr;
	osStatus_t res = osMessageQueueGet(uart6_rx_DMA_MessageQueue_Id, &uart6_rx_ndtr, NULL, timeout);				 						
	if (res == osOK)       		
	{	
//		PRINT_DBG("uart6 rx size %d\r\n", uart6_rx_ndtr);
		uint32_t size = fifo_out(&uart6_rxfifo, (void *)pData, uart6_rx_ndtr);
		HAL_UART_Receive_DMA(&huart6,uart6_rx_buffer,BUFFER_SIZE);	
//		PRINT_DBG("size = %d\r\n", size);		
		if(size != uart6_rx_ndtr)			
		{				
			PRINT_ERR("uatr6 rx fifo error\r\n");
			return 0;	
		}
		return size;		
	}
	return 0;
	
}

void UART_Rxhandle(UART_HandleTypeDef *huart, uint8_t *data)
{    
	uint32_t tmp_flag = 0;   
	uint32_t temp;
  UNUSED(huart); 
	tmp_flag =  __HAL_UART_GET_FLAG(huart,UART_FLAG_IDLE);
	if((tmp_flag != RESET))
		{
					
			__HAL_UART_CLEAR_IDLEFLAG(huart);			
			temp = huart->Instance->SR;
			temp = huart->Instance->DR;
			HAL_UART_AbortReceive(huart);				

			
			temp = huart->hdmarx->Instance->NDTR;
        
			temp = BUFFER_SIZE - temp;
			
			if(huart == &huart1)
			{
				static uint32_t uart1_index = 0;			
				uart1_rx_DMA_MessagePool[uart1_index] = temp;				
				osMessageQueuePut(uart1_rx_DMA_MessageQueue_Id, &uart1_rx_DMA_MessagePool[uart1_index], 0, 0);
				fifo_in(&uart1_rxfifo, (uint8_t *)data , uart1_rx_DMA_MessagePool[uart1_index]);
				set_returnzero(&uart1_index, uart1_rx_DMA_MESSAGE_POOL_SIZE);
			}
			else if(huart == &huart2)
			{
				static uint32_t uart2_index = 0;			
				uart2_rx_DMA_MessagePool[uart2_index] = temp;				
				osMessageQueuePut(uart2_rx_DMA_MessageQueue_Id, &uart2_rx_DMA_MessagePool[uart2_index], 0, 0);
				fifo_in(&uart2_rxfifo, (uint8_t *)data , uart2_rx_DMA_MessagePool[uart2_index]);
				set_returnzero(&uart2_index, uart2_rx_DMA_MESSAGE_POOL_SIZE);
			}
			else if(huart == &huart6)
			{
				static uint32_t uart6_index = 0;	
				uart6_rx_DMA_MessagePool[uart6_index] = temp;
				osMessageQueuePut(uart6_rx_DMA_MessageQueue_Id, &uart6_rx_DMA_MessagePool[uart6_index], 0, 0);
				fifo_in(&uart6_rxfifo, (uint8_t *)data, uart6_rx_DMA_MessagePool[uart6_index]);
				set_returnzero(&uart6_index, uart6_rx_DMA_MESSAGE_POOL_SIZE);
			}

		}
		
}

void USART1_IRQHandler(void)
{
	
  HAL_UART_IRQHandler(&huart1);

	UART_Rxhandle(&huart1, uart1_rx_buffer);
	
}

void USART2_IRQHandler(void)
{
	
  HAL_UART_IRQHandler(&huart2);	
	
	UART_Rxhandle(&huart2, uart2_rx_buffer);

}

void USART6_IRQHandler(void)
{
	
  HAL_UART_IRQHandler(&huart6);
		
	UART_Rxhandle(&huart6, uart6_rx_buffer);

}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
        osSemaphoreRelease(UART1_Semaphore);
    if (huart->Instance == USART2)
        osSemaphoreRelease(UART2_Semaphore);
    if (huart->Instance == USART6)
        osSemaphoreRelease(UART6_Semaphore);
}
