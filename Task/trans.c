#include "trans.h"
#include "trans_def.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
#include "debug.h"
#include "fifo.h"
#include "bigendian.h"
#include "esp8266.h"
#include "lorawan.h"
#include "string.h"
#include "list.h"
//#include "wifi.h"
#include "uart_server.h"
#include "lorawan_def.h"

extern osMessageQueueId_t lorawan_tx_MessageQueue_Id;
extern Tag_data TAG_DATA_BUFFER[];
extern uint8_t Anchor_number;
extern LOW_AnchorInfoTypeDef ANCHOR_INF_BUFFER[];
extern osMemoryPoolId_t list_MemPool; 
extern osMemoryPoolId_t tag_MemPool; 
extern osMemoryPoolId_t anchor_MemPool; 
extern uint8_t WiFi_ReadyFlag;
extern struct kfifo trans_tag_tx_kfifo;
uint64_t utc0_ts_timestamp;
uint32_t	match_id_flag;

osThreadId_t TRANS_Thread_Id;
osThreadAttr_t TRANS_Thread_Attr = {.priority = osPriorityNormal2, .stack_size = 512};

//osSemaphoreId_t UART6_Semaphore;
//osSemaphoreAttr_t UART6_Semaphore_Attr = {.name = "UART6_Semaphore"};
//osSemaphoreId_t tag_Semaphore;
//osSemaphoreAttr_t tag_Semaphore_Attr = {.name = "tag_Semaphore"};

osMessageQueueAttr_t anchor_MessageQueue_Attr = {.name = "anchor_MessageQueue"};
osMessageQueueId_t anchor_MessageQueue_Id;
osMessageQueueAttr_t tag_MessageQueue_Attr = {.name = "tag_MessageQueue"};
osMessageQueueId_t tag_MessageQueue_Id;

void TRANS_Thread(void *arg);

uint8_t trans_rx_buffer[512];
uint8_t trans_tx_buffer[512];
INST_MEASURE_INFO_SERVER_anchor_TypeDef anchor_info[15];
INST_MEASURE_INFO_SERVER_tag_TypeDef tag_id_info[50];

extern Station_P_Inf List_P;

extern osMessageQueueId_t uart6_rx_DMA_MessageQueue_Id;
extern struct kfifo uart6_rxfifo;
osThreadId_t TRANS_Init(void)
{
	
//		UART6_Semaphore = osSemaphoreNew(1, 0, &UART6_Semaphore_Attr);    
//		if (UART6_Semaphore == NULL)    
//		{        
//			PRINT_ERR("anchor semaphore create failed \r\n");   
//		}
		
		anchor_MessageQueue_Id = osMessageQueueNew(1, sizeof(uint16_t), &anchor_MessageQueue_Attr);
    if (anchor_MessageQueue_Id == NULL)
    {
        PRINT_ERR("anchor Message Queue creat failed \r\n");
    }

		tag_MessageQueue_Id = osMessageQueueNew(1, sizeof(uint16_t), &tag_MessageQueue_Attr);
    if (tag_MessageQueue_Id == NULL)
    {
        PRINT_ERR("tag Message Queue creat failed \r\n");
    }		
//		tag_Semaphore = osSemaphoreNew(1, 0, &tag_Semaphore_Attr);    
//		if (tag_Semaphore == NULL)    
//		{        
//			PRINT_ERR("tag semaphore create failed \r\n");   
//		}
		
    TRANS_Thread_Id = osThreadNew(TRANS_Thread, NULL, &TRANS_Thread_Attr);
    if (TRANS_Thread_Id == NULL)
    {
        PRINT_ERR("CTL Thread create failed \r\n");
        return NULL;
    }
    return TRANS_Thread_Id;
}


void TRANS_Thread(void *arg)
{
	osStatus_t res;	
	uint16_t tag_send_count = 2;
	uint32_t size;
	res = trans_device_init();
	uint8_t trans_state = 1;
	if(res == 1)
	{
		PRINT_ERR(" trans device init error\r\n");
	}
	res = inst_register_central();
	for(;;)
	{
		switch(trans_state)
		{
			case 1:
			{		
				res = trans_recv_handle();
				res = get_lora_thread_state();
				if( res == 1)
				{
					trans_state = 2;	
				}
					
			}
			break;
			case 2:
			{
				res = trans_recv_handle();
				
				size = fifo_used(&trans_tag_tx_kfifo);
//				size = trans_tag_tx_kfifo_used();
				if(size >= (sizeof(TRANS_TagInfoTypeDef) * tag_send_count))	
				{	
					PRINT_DBG("size : %u\r\n",size);	
					inst_tag_data_central(tag_send_count);	
				}
			}
			break;
			case 3:
			{
//				res = esp8266_recv(trans_rx_buffer,100);
//				if(res != 0)
//				{
//					uart1_send(trans_rx_buffer, res);
//				}			
			}
			break;


		}
		osDelay(50);
	}
	

}



int inst_register_central(void)
{	
	INST_REGISTER_CENTRAL_TypeDef *inst_register_central;
	inst_register_central = (INST_REGISTER_CENTRAL_TypeDef *)trans_tx_buffer;							
	inst_register_central->pkg_begin = 0xA0;							
	inst_register_central->content_len = sizeof(INST_REGISTER_CENTRAL_TypeDef) - sizeof(uint32_t);							
	mn2fun((uint16_t *)&inst_register_central->content_len, 2);							
	inst_register_central->op_id = 0x91;							
	inst_register_central->central_id = 5;							
	mn2fun((uint16_t *)&inst_register_central->central_id, 4);								
	inst_register_central->pkg_end = 0xA1;							
//	WiFi_Date_Transmit(WIFI_TXBUFFER, sizeof(INST_REGISTER_CENTRAL_TypeDef));	
	int status = esp8266_send((uint32_t *)trans_tx_buffer, sizeof(INST_REGISTER_CENTRAL_TypeDef));
	if(status != HAL_OK)
	{
		PRINT_ERR("inst register central error \r\n");
	}
	return status;
}

int inst_anchors_status_central(uint32_t *pdata, uint32_t *size)
{

	

	int status = esp8266_send(pdata, *size);
	if(status != HAL_OK)
	{
		PRINT_ERR("inst anchor status error \r\n");
	}
	return status;
}

int inst_tag_data_central(uint8_t trans_size)
{
//	uint8_t lorawan_tx_message;
	INST_TAG_DATA_CENTRAL_headTypeDef *inst_tag_data_central_head;
	INST_TAG_DATA_CENTRAL_tailTypeDef *inst_tag_data_central_tail;
	uint8_t inst_tag_data_central_buffer[64];
//	uint8_t  trans_size = 8;
						
//	if(0 == osMessageQueueGet(lorawan_tx_MessageQueue_Id, (uint8_t *)&lorawan_tx_message, 0, 10))						
	{							
//		PRINT_DBG("message %d\r\n",lorawan_tx_message);														
		inst_tag_data_central_head = (INST_TAG_DATA_CENTRAL_headTypeDef *)&trans_tx_buffer[0];							
		inst_tag_data_central_head->pkg_begin =0xA0;							
		inst_tag_data_central_head->content_len = 6 + trans_size * ( sizeof(TRANS_TagInfoTypeDef));							
		mn2fun((uint16_t *)&inst_tag_data_central_head->content_len, 2);							
		inst_tag_data_central_head->op_id = 0x82;							
		inst_tag_data_central_head->tag_count = trans_size;
		mn2fun((uint16_t *)&inst_tag_data_central_head->tag_count, 1);
		
		for (uint16_t i = 0; i < trans_size; i++)
		{
			uint16_t SIZE = fifo_out(get_trans_tag_tx_kfifo_struct(), inst_tag_data_central_buffer, sizeof(TRANS_TagInfoTypeDef));
//			uint16_t SIZE = trans_tag_tx_kfifo_out(inst_tag_data_central_buffer, sizeof(TRANS_TagInfoTypeDef));
			PRINT_DBG("SIZE %u\r\n",SIZE);
			memcpy(&trans_tx_buffer[sizeof(INST_TAG_DATA_CENTRAL_headTypeDef) + i * sizeof(TRANS_TagInfoTypeDef)], inst_tag_data_central_buffer, sizeof(TRANS_TagInfoTypeDef));
		}
		
		inst_tag_data_central_tail = (INST_TAG_DATA_CENTRAL_tailTypeDef *)&trans_tx_buffer[sizeof(INST_TAG_DATA_CENTRAL_headTypeDef) + trans_size * sizeof(TRANS_TagInfoTypeDef)];
		inst_tag_data_central_tail->match_id = (uint32_t)match_id_flag;
		mn2fun((uint16_t *)&inst_tag_data_central_tail->match_id, 4);
		inst_tag_data_central_tail->pkg_end =0xA1;													
		int status = esp8266_send((uint32_t *)trans_tx_buffer, sizeof(INST_TAG_DATA_CENTRAL_headTypeDef) + trans_size * sizeof(TRANS_TagInfoTypeDef) + sizeof(INST_TAG_DATA_CENTRAL_tailTypeDef));
		if(status != HAL_OK)
		{
			PRINT_ERR("inst tag central central error \r\n");
		}
		else{
			PRINT_DBG("tx succ %d \r\n",trans_size);
		}
		return status;						
	}
//	return 1;
}

int trans_recv_handle(void)
{
	INST_MEASURE_INFO_SERVER_head_TypeDef *inst_measure_info_server_head;
	uint32_t count = trans_recv(trans_rx_buffer, 10);
	if(count != 0)
	{
		inst_measure_info_server_head = (INST_MEASURE_INFO_SERVER_head_TypeDef *)trans_rx_buffer;
		PRINT_DBG("begin size %d\r\n", inst_measure_info_server_head->pkg_begin);
		if(inst_measure_info_server_head->pkg_begin == 0xA0)
		{
			mn2fun((uint16_t *)&inst_measure_info_server_head->content_len, 2);
			PRINT_DBG("content_len %d\r\n", inst_measure_info_server_head->content_len);
			if(inst_measure_info_server_head->content_len == count - sizeof(uint32_t))
			{
				
				mn2fun((uint16_t *)&inst_measure_info_server_head->op_id, 1);
				if(inst_measure_info_server_head->op_id == 0x81)
				{
					osStatus_t res = inst_measure_info_server(count);
					if(res == 1)
					{
						PRINT_ERR(" inst measure info server error\r\n");
					}
				}
				else if(inst_measure_info_server_head->op_id == 0x8E)
				{
					mn2fun((uint16_t *)&inst_measure_info_server_head->utc0_ts, 8);
					utc0_ts_timestamp = inst_measure_info_server_head->utc0_ts - (uint64_t)osKernelGetTickCount();
					if(trans_rx_buffer[sizeof(INST_MEASURE_INFO_SERVER_head_TypeDef)] == 0xA1)
					{
						PRINT_DBG("timestamp success\r\n");
					}
					else
					{
						PRINT_DBG("timestamp error\r\n");
					}
				}
				
			}
			else				
			{
			
				PRINT_ERR("trans recv length error\r\n");
				return 1;
			}
		}
		else
		{
			PRINT_ERR("trans recv head error\r\n");
			return 1;
		}
	}
	return 2;
}
int inst_measure_info_server(size_t size)
{
	osStatus_t res;	
	INST_MEASURE_INFO_SERVER_head_TypeDef *inst_measure_info_server_head;
	INST_MEASURE_INFO_SERVER_anchor_count_TypeDef *panchor_count;
	INST_MEASURE_INFO_SERVER_tag_count_TypeDef *ptag_count;
	INST_MEASURE_INFO_SERVER_tailTypeDef *pinst_measure_info_server_tail;	
			inst_measure_info_server_head = (INST_MEASURE_INFO_SERVER_head_TypeDef *)trans_rx_buffer;
			PRINT_DBG("begin size %d\r\n", inst_measure_info_server_head->pkg_begin);

//				mn2fun((uint16_t *)&inst_measure_info_server_head->content_len, 2);
				PRINT_DBG("content_len %d\r\n", inst_measure_info_server_head->content_len);

					
//					mn2fun((uint16_t *)&inst_measure_info_server_head->op_id, 1);
					if(inst_measure_info_server_head->op_id == 0x81)
					{
						mn2fun((uint16_t *)&inst_measure_info_server_head->utc0_ts, 8);
						utc0_ts_timestamp = inst_measure_info_server_head->utc0_ts - (uint64_t)osKernelGetTickCount();
						panchor_count = (INST_MEASURE_INFO_SERVER_anchor_count_TypeDef *)&trans_rx_buffer[sizeof(INST_MEASURE_INFO_SERVER_head_TypeDef)];
						memcpy(anchor_info, 
							&trans_rx_buffer[sizeof(INST_MEASURE_INFO_SERVER_head_TypeDef) + sizeof(INST_MEASURE_INFO_SERVER_anchor_count_TypeDef)], 
							panchor_count->anchor_count * sizeof(INST_MEASURE_INFO_SERVER_anchor_TypeDef));
							
//						if(res != osMessageQueuePut(anchor_MessageQueue_Id, &panchor_count->anchor_count, 0, 0))
//						{
//							PRINT_ERR("anchor messagequeue put error \r\n");
//							
//						}
							res = anchor_info_handle((uint8_t *)&panchor_count->anchor_count);
//							if(res == 0)
//								PRINT_DBG("anchor set success\r\n");
//							else{
//								PRINT_DBG("anchor set error\r\n");
						ptag_count = (INST_MEASURE_INFO_SERVER_tag_count_TypeDef *)&trans_rx_buffer[sizeof(INST_MEASURE_INFO_SERVER_head_TypeDef) + sizeof(uint8_t) + panchor_count->anchor_count * sizeof(INST_MEASURE_INFO_SERVER_anchor_TypeDef)];	
						memcpy(tag_id_info,
							&trans_rx_buffer[sizeof(INST_MEASURE_INFO_SERVER_head_TypeDef) + sizeof(uint16_t) + panchor_count->anchor_count * sizeof(INST_MEASURE_INFO_SERVER_anchor_TypeDef)],
							ptag_count->anchor_count * sizeof(INST_MEASURE_INFO_SERVER_tag_TypeDef));
						
//						if(res != osMessageQueuePut(tag_MessageQueue_Id, &ptag_count->anchor_count, 0, 0))
//						{
//							PRINT_ERR("tag messagequeue put error \r\n");
//							
//						}	
						res = tag_id_handle((uint8_t *)&ptag_count->anchor_count);
						if(res == 0)							
						{
							PRINT_DBG("tag set success\r\n");
							WiFi_ReadyFlag = 0;
						}
						else{
							PRINT_DBG("tag set error\r\n");
						}
						
						pinst_measure_info_server_tail = (INST_MEASURE_INFO_SERVER_tailTypeDef *)&trans_rx_buffer[size - sizeof(INST_MEASURE_INFO_SERVER_tailTypeDef)];
						mn2fun((uint16_t *)&pinst_measure_info_server_tail->match_id, 4);	
						match_id_flag  = pinst_measure_info_server_tail->match_id;
						if(pinst_measure_info_server_tail->pkg_end == 0xA1)
						{
							PRINT_OUT("inst measure info server rx success\r\n");
							return res;
						}									
		
	}	
	return res;
}

int tag_id_handle(uint8_t *count)
{
	void *ptag;
	tag_mempool_TypeDef pag_info;
	LIST_NodeTypeDef *node,*head;
	List_P.Tagnumber = *count;
	INST_MEASURE_INFO_SERVER_tag_TypeDef *pinst_measure_info_server_tag;						
	PRINT_DBG ("Tagnumber = 0x%02x\r\n",*count);						
	for (uint32_t i = 0; i < *count; i++)						
	{					
		pinst_measure_info_server_tag = (INST_MEASURE_INFO_SERVER_tag_TypeDef *)&tag_id_info[i];							
		mn2fun((uint16_t *)&pinst_measure_info_server_tag->tag_id, 4);					
		PRINT_DBG("tag_id = 0x%08x\r\n",(uint32_t)pinst_measure_info_server_tag->tag_id);	
		pag_info.tag_id = pinst_measure_info_server_tag->tag_id;
		pag_info.tag_packedid = 0;
		ptag = osMemoryPoolAlloc(tag_MemPool, 100);		
		if(ptag == NULL)
		{
			PRINT_DBG("memory tag pool error \r\n");
			return 1;
		}		
		node = osMemoryPoolAlloc(list_MemPool, 100);							
		if (ptag != NULL && node != NULL)							
		{								
			memcpy(ptag, (uint32_t *)&pag_info, sizeof(tag_mempool_TypeDef));								
			node->data = ptag;					
			if (i == 0)								
			{									
				head = node;									
				LIST_Init(head);
			}								
			else									
			LIST_AddTail(head, node);							
		}							
		else								
		PRINT_DBG("memory pool error \r\n");						
	}									
	List_P.p1 = node;

	return 0;	
}

int anchor_info_handle(uint8_t *count)
{
	void *panchor;
	LIST_NodeTypeDef *node,*head;
	INST_MEASURE_INFO_SERVER_anchor_TypeDef *panchor_inf;
	PRINT_DBG("anchor count = 0x%02x\r\n",*count);
	for(uint32_t i = 0; i < *count; i++ )
	{
//		PRINT_DBG("anchor size = 0x%02x\r\n",*count);
		panchor_inf = (INST_MEASURE_INFO_SERVER_anchor_TypeDef *)&anchor_info[i];
//							anchor_inf->anchor_id
		mn2fun((uint16_t *)&panchor_inf->anchor_id, 4);
		mn2fun((uint16_t *)&panchor_inf->anchor_x, 2);
		mn2fun((uint16_t *)&panchor_inf->anchor_y, 2);
		mn2fun((uint16_t *)&panchor_inf->anchor_z, 2);
		
////							tag.id = tag_idinf->tag_id;
////							tag.packetId = 0;
		ANCHOR_INF_BUFFER[i].AnchorId = (uint32_t)panchor_inf->anchor_id;
		ANCHOR_INF_BUFFER[i].Coor_x = (uint16_t)panchor_inf->anchor_x;
		ANCHOR_INF_BUFFER[i].Coor_y = (uint16_t)panchor_inf->anchor_y;
		ANCHOR_INF_BUFFER[i].Coor_z = (uint16_t)panchor_inf->anchor_z;
		
		Anchor_number = Anchor_number + 1;
		
		PRINT_DBG("anchor_id = %08d  anchor_x = %04d  anchor_y = %04d  anchor_z = %04d\r\n",
		(uint32_t)panchor_inf->anchor_id, 
		(int16_t)panchor_inf->anchor_x, 
		(int16_t)panchor_inf->anchor_y, 
		(int16_t)panchor_inf->anchor_z);

		panchor = osMemoryPoolAlloc(anchor_MemPool, 100);
		if(panchor == NULL)
		{
			PRINT_DBG("memory LOW_AnchorMemoryPoolId pool error \r\n");
			return 1;
		}
		node = osMemoryPoolAlloc(list_MemPool, 100);
		if (panchor != NULL && node != NULL)
		{
			memcpy(panchor, panchor_inf, sizeof(INST_MEASURE_INFO_SERVER_anchor_TypeDef));
			node->data = panchor;
			if (i == 0)
			{
				head = node;
				LIST_Init(head);
			}
			else
				LIST_AddTail(head, node);
		}
		else
		{
			PRINT_DBG("memory pool error \r\n");
			return 1;
		}								
	}
//	List_P.p2 = node;
	return 0;
}

int trans_device_init(void)
{
osStatus_t res = esp8266_server_init();
	if(res == 1)
	{
		PRINT_ERR(" cc3200 init error\r\n");
	}
}
	
int trans_send(const void *pData, size_t size)
{

	return esp8266_send(pData, size);
}
int trans_recv(const void *pData, size_t timeout)
{

	return esp8266_recv(pData, timeout);
}

uint64_t get_utc_timestamp(void)
{
	
	return utc0_ts_timestamp;
}
