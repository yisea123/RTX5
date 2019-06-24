#ifndef __TRANS_H
#define __TRANS_H
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
#include "trans_def.h"

int trans_recv_handle(void);
int inst_register_central(void);
int inst_anchors_status_central(uint32_t *pdata, uint32_t *size);
int inst_tag_data_central(uint8_t lorawan_tx_message);
int inst_measure_info_server(size_t size);
int tag_id_handle(uint8_t *count);
int anchor_info_handle(uint8_t *count);


uint64_t get_utc_timestamp(void);

osThreadId_t TRANS_Init(void);
int trans_device_init(void);
int trans_send(const void *pData, size_t size);
int trans_recv(const void *pData, size_t timeout);
#endif
