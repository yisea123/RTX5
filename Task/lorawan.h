#ifndef __LORAWAN_H
#define __LORAWAN_H

#include "cmsis_os2.h"

//__packed typedef struct
//{
//	uint32_t id;
//	uint64_t timestamp;
//	uint8_t batteryPercentage;
//	uint8_t packetId;
//	int8_t dataNumber;
//} TagResponseHeadTypeDef;

__packed typedef struct
{
	int16_t pos_x;
	int16_t pos_y;
	int16_t pos_z;
	int8_t speed_x;
	int8_t speed_y;
	int8_t speed_z;
	int8_t acc_x;
	int8_t acc_y;
	int8_t acc_z;
//	uint64_t timestamp;
} TagInfoTypeDef;



#define LOW_THREAD_PRIORITY osPriorityNormal	
#define TAG_DATA_FIFO_BUFFER_SIZE  512
int get_lora_thread_state(void);
void* get_trans_tag_tx_kfifo_struct(void);
size_t trans_tag_tx_kfifo_out(uint8_t *dst, size_t size);
size_t trans_tag_tx_kfifo_used(void);

int set_anchor_inf(uint8_t *pData);
int lora_recv_taginf(uint8_t *pData, uint32_t *response_id);


osThreadId_t LOW_Init(void);
void LOW_DeInit(void);

#endif
