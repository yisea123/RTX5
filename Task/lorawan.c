#include "lorawan.h"
#include "cmsis_os2.h"
#include "string.h"
#include "wifi.h"
#include "tfcard.h"
#include "bq24210.h"
#include "bq27542.h"
#include "sx1280.h"
//#include "esp8266.h"
#include "trans.h"
#include "config.h"
#include "debug.h"
#include "string.h"
#include "fifo.h"
#include "list.h"
#include "bigendian.h"
#include "lorawan_def.h"

#define LOW_THREAD_FLAG_STOP 0x02u

//#define LOW_DBG
#ifndef LOW_DBG
#define PRINT_LOW_DEG(fmt,arg...)            printf(fmt, ##arg)
#else
#define PRINT_LOW_DEG(fmt,arg...)            {}
#endif
#define TAG_DATA_BUFFER_SIZE 300
#define ANXHOR_INF_BUFFER_SIZE 30

uint8_t data_count_add = 0;
Tag_data TAG_DATA_BUFFER[TAG_DATA_BUFFER_SIZE];
uint8_t Anchor_number = 0;
LOW_AnchorInfoTypeDef ANCHOR_INF_BUFFER[ANXHOR_INF_BUFFER_SIZE];

static osThreadId_t LOW_Thread_Id;
uint8_t WiFi_ReadyFlag = 1;
static uint8_t lorRxBuffer[256];
static uint8_t lorTxBuffer[256];

//struct kfifo TAG_DATA_FIFO, TAG_DATA_TXFIFO;
//uint8_t TAG_DATA_FIFO_BUFFER[TAG_DATA_FIFO_BUFFER_SIZE];
//uint8_t TAG_DATA_FIFO_TXBUFFER[TAG_DATA_FIFO_BUFFER_SIZE];

//extern struct kfifo WIFI_FifoTx, WIFI_FifoRx;

//extern osMemoryPoolId_t LOW_TagMemoryPoolId;
//extern osMemoryPoolId_t LOW_TagDataMemoryPoolId;
//extern osMemoryPoolId_t LOW_ListNodeMemoryPoolId;

uint8_t trans_tag_tx_buffer[sizeof(TRANS_TagInfoTypeDef)];
struct kfifo trans_tag_tx_kfifo;
uint8_t trans_tag_tx_kfifo_buffer[2048];

 Station_P_Inf List_P;
//extern LIST_NodeTypeDef *wifi_tag_node;
//extern LIST_NodeTypeDef *wifi_tag_data_node;
extern uint64_t utc0_ts_timestamp;
extern uint8_t data_tx_flag;
//extern uint8_t Tagnumber ;	
INST_DATA_TypeDef tag_data;
typedef enum
{
	LOW_STATE_INIT,
	LOW_STATE_WAITSYNC,
	LOW_STATE_GO,
	LOW_STATE_TEST,
//	LOW_STATE_WIFI_TEST,
}LOW_StateTypeDef;

typedef enum
{
	REQ_REQ,
	REQ_ACK,
}REQ_StateTypeDef;

typedef enum
{
	LOW_PACKET_BROADCAST = 0x00,
	LOW_PACKET_TAG_REQ = 0x40,
	LOW_PACKET_TAG_ACK = 0xc0
} LOW_PacketType;

__packed typedef struct
{
	uint32_t id;
	uint8_t packetId;
	uint64_t utc;	
	uint8_t *reserved;
} TagRequestTypeDef;

__packed typedef struct
{
	uint64_t utc;
	uint8_t headId;
	uint8_t numberOfSlots;
	uint8_t slotDuration; //in ms
	uint8_t numberOfAnchors;
	uint16_t samplingInterval;
} SyncHeadTypeDef;

__packed typedef struct
{
	uint32_t id;
	uint64_t timestamp;
	uint8_t batteryPercentage;
	uint8_t packetId;
	int8_t dataNumber;
} TagResponseHeadTypeDef;

osThreadAttr_t LOW_Thread_Attr = {.priority = LOW_THREAD_PRIORITY, .stack_size = 1024};

static void LOW_Thread(void *argument);



uint32_t list_MemoryPool[(sizeof(LIST_NodeTypeDef)* 80) / 4];
osMemoryPoolAttr_t list_MemoryPoolAttr = {.mp_mem = list_MemoryPool, .mp_size = sizeof(LIST_NodeTypeDef) * 80};
osMemoryPoolId_t list_MemPool; 

uint32_t tag_MemoryPool[(sizeof(tag_mempool_TypeDef) * 60) / 4];
osMemoryPoolAttr_t tag_MemoryPoolAttr = {.mp_mem = tag_MemoryPool, .mp_size = sizeof(tag_mempool_TypeDef) * 60};
osMemoryPoolId_t tag_MemPool; 

uint32_t anchor_MemoryPool[(sizeof(LOW_AnchorInfoTypeDef) * 20) / 4];
osMemoryPoolAttr_t anchor_MemoryPoolAttr = {.mp_mem = anchor_MemoryPool, .mp_size = sizeof(LOW_AnchorInfoTypeDef) * 20};
osMemoryPoolId_t anchor_MemPool; 

uint32_t lorawan_tx_MessagePool[sizeof(uint16_t) * 10 / 4];
osMessageQueueAttr_t lorawan_tx_MessageQueue_Attr = {.name = "lorawan_tx_MessageQueue", .mq_mem = (void *)lorawan_tx_MessagePool, .mq_size = sizeof(uint16_t) * 10};
osMessageQueueId_t lorawan_tx_MessageQueue_Id;

osThreadId_t LOW_Init(void)
{
	LOR_Init();	
	
	list_MemPool = osMemoryPoolNew(80, sizeof(LIST_NodeTypeDef), &list_MemoryPoolAttr);
	if (list_MemPool == NULL) 
	{
		PRINT_ERR("list mempool created error\r\n"); // MemPool object not created, handle failure
	}
	
	tag_MemPool = osMemoryPoolNew(20, sizeof(tag_mempool_TypeDef), &tag_MemoryPoolAttr);
	if (tag_MemPool == NULL) 
	{
		PRINT_ERR("tag mempool created error\r\n"); // MemPool object not created, handle failure
	}

	anchor_MemPool = osMemoryPoolNew(10, sizeof(LOW_AnchorInfoTypeDef), &anchor_MemoryPoolAttr);
	if (anchor_MemPool == NULL) 
	{
		PRINT_ERR("anchor mempool created error\r\n"); // MemPool object not created, handle failure
	}
	
	if (fifo_init(&trans_tag_tx_kfifo, trans_tag_tx_kfifo_buffer, 2048))
	{
			PRINT_ERR("trans tag tx fifo init failed \r\n");
//			goto exit;
	}
	
	lorawan_tx_MessageQueue_Id = osMessageQueueNew(1, sizeof(uint16_t), &lorawan_tx_MessageQueue_Attr);
    if (lorawan_tx_MessageQueue_Id == NULL)
    {
        PRINT_ERR("UART Rx Message Queue creat failed \r\n");
//        goto exit;
    }
	
	LOW_Thread_Id = osThreadNew(LOW_Thread, NULL, &LOW_Thread_Attr);
	if (LOW_Thread_Id != NULL)
		return LOW_Thread_Id;
	else
	{
		PRINT_ERR("LOW Thread create failed \r\n");
		return NULL;
	}
}

static void LOW_Thread(void *argument)
{		
	LOW_StateTypeDef LOW_state;
	REQ_StateTypeDef REQ_state;
		
	TagRequestTypeDef *pTagRequest;	
	TagResponseHeadTypeDef *pTagResponse;
	tag_mempool_TypeDef *wifi_p_tag;		
	LIST_NodeTypeDef *wifi_tag_node;
	LIST_NodeTypeDef *set_flag_node;
	uint32_t first_id;
	
	LOR_Event event;
	uint32_t size;
	LOW_state = LOW_STATE_INIT;
	for (;;)		
	{			
		while(WiFi_ReadyFlag != 0)
		{osDelay (10);}
		switch(LOW_state)
		{
			case LOW_STATE_INIT:
			{				
				PRINT_LOW_DEG("low state init\r\n");
				REQ_state = REQ_REQ;
				wifi_tag_node = List_P.p1;	
				
				set_flag_node = osMemoryPoolAlloc(list_MemPool, 100);
				LIST_Add(wifi_tag_node,set_flag_node);
				
				LOW_state = LOW_STATE_WAITSYNC;
				
			break;
			}
			case LOW_STATE_WAITSYNC:
			{
				set_anchor_inf((uint8_t *)&lorTxBuffer);
//				wifi_tag_node = wifi_tag_node->next;
			}
			LOW_state = LOW_STATE_GO;	
			break;	
			case LOW_STATE_GO:
			{
				switch(REQ_state)
				{
					case REQ_REQ:
					{
						
						wifi_p_tag = wifi_tag_node->data;																								
						lorTxBuffer[0] = LOW_PACKET_TAG_REQ;
						pTagRequest = (TagRequestTypeDef *)(lorTxBuffer + 1);
						pTagRequest->id = wifi_p_tag->tag_id;
						pTagRequest->utc = (uint64_t)osKernelGetTickCount();	
						
						pTagRequest->packetId = wifi_p_tag->tag_packedid;		
						PRINT_DBG("request id = %u  packedif = %u\r\n",(uint32_t)pTagRequest->id, pTagRequest->packetId);
						event = LOR_Transmit(lorTxBuffer, 1 + sizeof(TagRequestTypeDef),0xFFFF);
						if(event != 0)
						{
							PRINT_DBG("lor tx error\r\n");
						}
						wifi_tag_node = wifi_tag_node->next;
					}
					case REQ_ACK:
					{
						event = lora_recv_taginf(lorRxBuffer, (uint32_t *)&first_id);	
						if(event ==LOR_EVENT_OK )
						{
							PRINT_DBG("pTagResponse.id %u\r\n",first_id);
							if(first_id == pTagRequest->id)
							{
								wifi_p_tag->tag_packedid++;	
								PRINT_DBG("pTagResponse.id %u\r\n",pTagResponse->id);
							}						
						}							

						if(wifi_tag_node == set_flag_node)
						{
//							PRINT_DBG("pTagRequest id : %u\r\n",pTagRequest->id);
							LOW_state = LOW_STATE_WAITSYNC;
							break;
						}						
					}
				}			
			}	
//			LOW_state = LOW_STATE_TEST;			
			break;
			case LOW_STATE_TEST:
			{
				
				event = LOR_Receive(lorRxBuffer, &size, LOW_WAIT_SYNC_TIMEOUT);	
				if(LOR_EVENT_OK == event)
				{
					PRINT_LOW_DEG("Rx good\r\n");
				}
				osDelay(35);			
			}				
	}


	}	
}

//int get_list_next_data(LIST_NodeTypeDef *node, void *data)
//{
//	data = node->next->data;
//}

int set_anchor_inf(uint8_t *pData)
{
	SyncHeadTypeDef *pTagsynchead;
	*pData = (uint8_t)LOW_PACKET_BROADCAST;
	pTagsynchead = (SyncHeadTypeDef *)(pData + 1);
	pTagsynchead->headId = LOW_PACKET_BROADCAST;
	pTagsynchead->numberOfAnchors = Anchor_number;
	pTagsynchead->numberOfSlots = 0;
	pTagsynchead->slotDuration = 0;
	pTagsynchead->utc = (uint64_t)osKernelGetTickCount() + utc0_ts_timestamp;
	pTagsynchead->samplingInterval = SamplingInterval;
//				PRINT_DBG("timestamp %llu\r\n",utc0_ts_timestamp);
	memcpy((uint8_t *)(pData +1 + sizeof(SyncHeadTypeDef)), &ANCHOR_INF_BUFFER[0], Anchor_number * sizeof(LOW_AnchorInfoTypeDef));
//				PRINT_LOW_DEG("Enter anchor pos init  loraTxbuffer[0] = %d  Anchor_number = %d\r\n",lorTxBuffer[0], (Anchor_number * sizeof(LOW_AnchorInfoTypeDef)));									
	LOR_Event event = LOR_Transmit((uint8_t *)pData, 1 + sizeof(SyncHeadTypeDef) + Anchor_number * sizeof(LOW_AnchorInfoTypeDef), 0xFFFF);
	if(event != LOR_EVENT_OK)
	{
		PRINT_LOW_DEG("Anchor fail\r\n");
	}
	return event;
}

int lora_recv_taginf(uint8_t *pData, uint32_t *response_id)
{
	uint32_t size;
	TagResponseHeadTypeDef *pTagResponse;
	TagInfoTypeDef *pTagInfo;
	TRANS_TagInfoTypeDef *ptranstag;	
	LOR_Event event = LOR_Receive(lorRxBuffer, &size, LOW_WAIT_SYNC_TIMEOUT);
	if(event == LOR_EVENT_OK)
	{
		if(lorRxBuffer[0] == LOW_PACKET_TAG_ACK)
		{				
			pTagResponse = (TagResponseHeadTypeDef *)(lorRxBuffer + 1);														
			*response_id = (uint32_t )pTagResponse->id;																																							
			for(uint32_t i = 0 ; i < pTagResponse->dataNumber ; i++)
			{																										
				TAG_DATA_BUFFER[data_count_add].timestamp = pTagResponse->timestamp + i * SamplingInterval;
				TAG_DATA_BUFFER[data_count_add].id = pTagResponse->id;
				TAG_DATA_BUFFER[data_count_add].batteryPercentage = pTagResponse->batteryPercentage;
				
				 
				pTagInfo = (TagInfoTypeDef *)(lorRxBuffer + 1 + sizeof(TagResponseHeadTypeDef) + sizeof(TagInfoTypeDef) * i);
				PRINT_DBG("timestamp  %llu  id  %08d  battery  %02d  pox x  %04d  pox y  %04d  pox z  %04d  speed x  %d  speed y  %d  speed z  %d  acc x  %d  acc y  %d  acc z  %d\r\n",
				(uint64_t)TAG_DATA_BUFFER[data_count_add].timestamp,
				(uint32_t)pTagResponse->id,
				(uint8_t)pTagResponse->batteryPercentage,
				(int16_t)pTagInfo->pos_x, 
				(int16_t)pTagInfo->pos_y, 
				(int16_t)pTagInfo->pos_z, 
				(int8_t)pTagInfo->speed_x, 
				(int8_t)pTagInfo->speed_y, 
				(int8_t)pTagInfo->speed_z, 
				(int8_t)pTagInfo->acc_x, 
				(int8_t)pTagInfo->acc_y, 
				(int8_t)pTagInfo->acc_z);	
				
				ptranstag = (TRANS_TagInfoTypeDef *)trans_tag_tx_buffer;
				
				ptranstag->timestamp = pTagResponse->timestamp + i * SamplingInterval;  
				ptranstag->id = pTagResponse->id;
				ptranstag->batteryPercentage = pTagResponse->batteryPercentage;
				ptranstag->pos_x = (int16_t)pTagInfo->pos_x;
				ptranstag->pos_y = (int16_t)pTagInfo->pos_y;
				ptranstag->pos_z = (int16_t)pTagInfo->pos_z;
				ptranstag->speed_x = (int16_t)pTagInfo->speed_x;
				ptranstag->speed_y = (int16_t)pTagInfo->speed_y;
				ptranstag->speed_z = (int16_t)pTagInfo->speed_z;
				ptranstag->acc_x = (int16_t)pTagInfo->acc_x;
				ptranstag->acc_y = (int16_t)pTagInfo->acc_y;
				ptranstag->acc_z = (int16_t)pTagInfo->acc_z;
				

				
				mn2fun((uint16_t *)&ptranstag->timestamp,8);
				mn2fun((uint16_t *)&ptranstag->id,4);
				mn2fun((uint16_t *)&ptranstag->batteryPercentage,1);
				mn2fun((uint16_t *)&ptranstag->pos_x,2);
				mn2fun((uint16_t *)&ptranstag->pos_y,2);
				mn2fun((uint16_t *)&ptranstag->pos_z,2);
				
				size = fifo_in(get_trans_tag_tx_kfifo_struct(), (uint8_t *)ptranstag, sizeof(TRANS_TagInfoTypeDef));
				PRINT_DBG("fifo lora used %u\r\n", fifo_used(get_trans_tag_tx_kfifo_struct()));
				if(size < sizeof(trans_tag_tx_buffer))
				{
					TRANS_TagInfoTypeDef ptranstag_1;
					fifo_out(get_trans_tag_tx_kfifo_struct(), (uint8_t *)&ptranstag_1, sizeof(TRANS_TagInfoTypeDef));
					fifo_in(get_trans_tag_tx_kfifo_struct(), (uint8_t *)(ptranstag + size), sizeof(TRANS_TagInfoTypeDef) - size);
				}
				
					
			}	
		
		}
	}
	return event;
}

void* get_trans_tag_tx_kfifo_struct(void)
{
	
	return &trans_tag_tx_kfifo;
}

int get_lora_thread_state(void)
{
	
	return osThreadGetState(LOW_Thread_Id);
}

size_t trans_tag_tx_kfifo_used(void)
{

	return fifo_used(&trans_tag_tx_kfifo);
}

size_t trans_tag_tx_kfifo_out(uint8_t *dst, size_t size)
{

	return fifo_out(&trans_tag_tx_kfifo, dst, (uint32_t)size);
}
