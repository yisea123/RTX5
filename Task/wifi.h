#ifndef __WIFI_H
#define __WIFI_H

#include "cmsis_os2.h"

#define WIFI_THREAD_FLAG_STOP 0x02u
#define WIFI_FIFO_TX_SIZE 4096
#define WIFI_FIFO_RX_SIZE 1024
#define DATA_FIFO_RX_SIZE 1024
#define BUFFER_SIZE 300

typedef enum{
    ESP_Event_OK,
    ESP_Event_ERROR,
    ESP_Event_TIMEOUT,
}ESP_EventTypeDef;

typedef enum{
    WIFI_Event_OK,
    WIFI_Event_ERROR,
    WIFI_Event_TIMEOUT,
}WIFI_EventTypeDef;

osThreadId_t WIFI_Init(void);
//void WIFI_TX(uint8_t *data);
//void WIFI_RX(uint8_t *data);

#endif
