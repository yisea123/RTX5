#ifndef __ESP8266_DEF_H
#define __ESP8266_DEF_H
#include "stm32f4xx_hal.h"

uint8_t READY_TT[] = "ready\r\n";
uint8_t AT_UART[] = "AT+UART=115200,8,1,0,0\r\n";
uint8_t AT_CWMOOE[] = "AT+CWMODE=1\r\n";
uint8_t AT_CWJAP[] = "AT+CWJAP=\"123123\",\"12345678\" \r\n";
uint8_t AT_CIPMUX[] = "AT+CIPMUX=0\r\n";
uint8_t WIFI_GOT_IP[] = "WIFI GOT IP\r\n";
uint8_t AT_CIPSTART[] = "AT+CIPSTART=\"TCP\",\"192.168.137.1\",8001\r\n";
//uint8_t AT_CIPSTART[] = "AT+CIPSTART=\"TCP\",\"120.76.210.158\",4199\r\n";
uint8_t AT_CIPMODE[] = "AT+CIPMODE=1\r\n";
uint8_t AT_CIPSEND[] = "AT+CIPSEND\r\n";


#endif
