#ifndef __CC3200_H
#define __CC3200_H
//#include "string.h"
#include "stm32f4xx_hal.h"

//#define huart6  hcc3200

//uint8_t  AT_Setmode[] = "+++";
//uint8_t  AT_ROLE[] = "AT+ROLE=STA\r\n";
//uint8_t  AT_STA[] = "AT+STA=WIFI,2,95008cqa\r\n";
////uint8_t  AT_APIP_[] = "AT+APIP=?";
//uint8_t  AT_RST[] = "AT+RST\r\n";
////uint8_t  AT_ATAIP[] = "AT+STAIP=?";
//uint8_t  AT_SOCK[] = "AT+SOCK=TCP,CLIENT,192.168.137.1,8001,8887\r\n";
////uint8_t  AT_KEEPALIVE[] = "AT+KEEPALIVE=?";
//uint8_t  AT_KEEPALIVE[] = "AT+KEEPALIVE=OFF,50,5,0011223344\r\n";
//uint8_t  AT_REGPKT_OFF[] = "AT+REGPKT=OFF\r\n";
////uint8_t  AT_REGPKT[] = "AT+REGPKT=?";
//uint8_t  AT_REGPKT[] = "AT+REGPKT=ON,9,a000059100000014a1\r\n";


int cc3200_server_init(void);
int cc3200_send(const void *pData, size_t size);
int cc3200_recv(const void *pData, size_t timeout);

//int cc3200_enterat(void);

int cc3200_enterat_1(void);
int cc3200_enterat_2(void);
int cc3200_sendat(const void *pData, size_t Size);
#endif 
