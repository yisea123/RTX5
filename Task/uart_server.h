#ifndef __UART_SERVER_H
#define __UART_SERVER_H
#include "cmsis_os2.h"

#define BUFFER_SIZE 512

osThreadId_t UART_SERVER_Init(void);

int uart_server_rx_HardwareInit(void);
void uart6_receive_dma(void);
int uart1_send(const void *pData, size_t size);
int uart1_recv(const void *pData, size_t timeout);
int uart2_send(const void *pData, size_t size);
int uart2_recv(const void *pData, size_t timeout);
int uart6_send(const void *pData, size_t size);
int uart6_recv(const void *pData, size_t timeout);
#endif

