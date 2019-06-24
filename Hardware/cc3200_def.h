#ifndef __CC3200_DEF_H
#define __CC3200_DEF_H
#include "stm32f4xx_hal.h"

uint8_t  AT_Setmode_1[] = "+++";
uint8_t  AT_Setmode_2[] = "a";
uint8_t  AT_WMODE[] = "AT+WMODE=STA\r\n";
uint8_t  AT_WSTA[] = "AT+WSTA=WIFI,95008cqa\r\n";
uint8_t  AT_WANN[] = "AT+WANN=DHCP\r\n";
uint8_t  AT_WKMOD[] = "AT+WKMOD=TRANS\r\n";
uint8_t  AT_SOCKA[] = "AT+SOCKA=TCPC,192.168.137.1,8001\r\n";
//uint8_t  AT_SOCKA[] = "AT+SOCKA=TCPC,192.168.137.1,8001\r\n";
//uint8_t  AT_REGENA[] = "AT+REGENA=USR,FIRST\r\n";
uint8_t  AT_REGENA[] = "AT+REGENA=OFF\r\n";
uint8_t  AT_REGUSR[] = "AT+REGUSR=0xa000059100000001a1\r\n";


uint8_t  AT_Z[] = "AT+Z\r\n";
uint8_t  AT_ENTM[] = "AT+ENTM\r\n";
uint8_t  AT_RELD[] = "AT+RELD\r\n";


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

#endif
