#ifndef __ESP8266_H
#define __ESP8266_H

#include "stm32f4xx_hal.h"
int esp8266_server_init(void);

int esp8266_send(const void *pData, size_t size);
int esp8266_recv(const void *pData, size_t timeout);

int esp8266_sendat(const void *pData, size_t Size);

#endif
