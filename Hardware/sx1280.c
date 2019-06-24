#include "sx1280.h"
#include "cmsis_os2.h"
#include "debug.h"
#include "config.h"

//#define LOR_RESET_L() HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET)
//#define LOR_RESET_H() HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET)
//#define LOR_CS_L() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET)
//#define LOR_CS_H() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET)
//#define LOR_BUSY() HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_3)

#define LOR_RESET_L() HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET)
#define LOR_RESET_H() HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET)
#define LOR_CS_L() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET)
#define LOR_CS_H() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET)
#define LOR_BUSY() HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_2)

#define LOR_PBLE_LEN_MANT ((uint8_t)3u)
#define LOR_PBLE_LEN_EXP ((uint8_t)2u) //preamble length = LOR_PBLE_LEN_MANT*2^(LOR_PBLE_LEN_EXP)
#define LOR_IMPLICIT_HEADER ((uint8_t)0x80u)
#define LOR_EXPLICIT_HEADER ((uint8_t)0x00u)
#define LOR_CRC_ENABLE ((uint8_t)0x20u)
#define LOR_IQ_STD ((uint8_t)0x40u)
#define LORA_IQ_INVERTED 0x00

#define LOR_IRQMASK_LOW_TXDONE ((uint8_t)0x01u)
#define LOR_IRQMASK_LOW_RXDONE ((uint8_t)0x02u)
#define LOR_IRQMASK_SYNC_WORD_ERROR ((uint8_t)0x08u)
#define LOR_IRQMASK_HEADER_ERROR ((uint8_t)0x20u)
#define LOR_IRQMASK_CRC_ERROR ((uint8_t)0x40u)
#define LOR_IRQMASK_HIGH_RXTXTIMEOUT ((uint8_t)0x40u)
#define LOR_IRQMASK_LOW_CRCERROR ((uint8_t)0x40u)

#define LOR_TX_BUFFER_START 0x00

extern osMutexId_t spi2_mutex;
const osSemaphoreAttr_t LOR_Semaphore_Attr = {.name = "LOR_Semaphore"};
extern SPI_HandleTypeDef hspi2;

static uint8_t txBuffer[256];
static uint8_t rxBuffer[256];

//static osThreadId LOR_thread_id;
osSemaphoreId_t LOR_Semaphore;

//void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
//{
//	if (GPIO_Pin == GPIO_PIN_3)	//sx1280 dio irq
//	{
//		LOR_EXTI_Callback();
//	}
//}

static void LOR_SetCmd(uint32_t size)
{
    osStatus_t res;
	while (LOR_BUSY() == GPIO_PIN_SET)
	{
		osDelay(1);
//		HAL_Delay(1);
	}
	//osMutexWait(spi2_mutex, osWaitForever);
//    res = osMutexAcquire(spi2_mutex,500);
//    if(res != osOK)
//    {
//        PRINT_DBG("LOR spi2 mutex wait timeout \r\n");
//        return;
//    }
//    else
//    {
        LOR_CS_L();
        HAL_SPI_Transmit(&hspi2, txBuffer, size, HAL_MAX_DELAY);
        LOR_CS_H();
//        osMutexRelease(spi2_mutex);
//    }
}

static void LOR_GetCmd(uint32_t size)
{
//    osStatus_t res;
	while (LOR_BUSY() == GPIO_PIN_SET)
	{
		osDelay(1);
//		HAL_Delay(1);		
	}
    //osMutexWait(spi2_mutex, osWaitForever);
//    res = osMutexAcquire(spi2_mutex,500);
//    if(res != osOK)
//    {
//        PRINT_DBG("LOR spi2 mutex wait timeout \r\n");
//        return;
//    }
//    else
//    {

        LOR_CS_L();
        HAL_SPI_TransmitReceive(&hspi2, txBuffer, rxBuffer, size, HAL_MAX_DELAY);
        LOR_CS_H();
//        osMutexRelease(spi2_mutex);
//    }
}

static void LOR_WriteRegister(uint16_t regAddress, uint8_t value)
{
//    osStatus_t res;
	txBuffer[0] = 0x18;
	txBuffer[1] = (uint8_t)(regAddress >> 8);
	txBuffer[2] = (uint8_t)regAddress;
	txBuffer[3] = value;
	//osMutexWait(spi2_mutex, osWaitForever);
//    res = osMutexAcquire(spi2_mutex,500);
//    if(res != osOK)
//    {
//        PRINT_DBG("LOR spi2 mutex wait timeout \r\n");
//        return;
//    }
//    else
//    {    
        while (LOR_BUSY() == GPIO_PIN_SET);
        LOR_CS_L();
        HAL_SPI_TransmitReceive(&hspi2, txBuffer, rxBuffer, 4, HAL_MAX_DELAY);
        LOR_CS_H();
//        osMutexRelease(spi2_mutex);
//    }
}

static LOR_Event LOR_GetStatus(void)
{
	txBuffer[0] = 0xc0;
	LOR_GetCmd(2);
	return LOR_EVENT_OK;
}

static LOR_Event LOR_SetPacketType(void)
{
	txBuffer[0] = 0x8a;
	txBuffer[1] = 0x01;
	LOR_SetCmd(2);
	return LOR_EVENT_OK;
}

LOR_Event LOR_SetStandbyXTALMode(void)
{
	txBuffer[0] = 0x80;
	txBuffer[1] = 0x01;
	LOR_SetCmd(2);
	return LOR_EVENT_OK;
}

static LOR_Event LOR_SetRfFrequency(void)
{
	txBuffer[0] = 0x86;
	txBuffer[1] = LOR_FREQ_HB;
	txBuffer[2] = LOR_FREQ_MB;
	txBuffer[3] = LOR_FREQ_LB;
	LOR_SetCmd(4);
	return LOR_EVENT_OK;
}

static LOR_Event LOR_SetModulationParams(void)
{
	txBuffer[0] = 0x8b;
	txBuffer[1] = LOR_SF_8;
	txBuffer[2] = LOR_BW_1600;
	txBuffer[3] = LOR_CR_4_5;
	LOR_SetCmd(4);
	LOR_WriteRegister(0x925, 0x1e);
	return LOR_EVENT_OK;
}

static LOR_Event LOR_SetBufferBaseAddress(void)
{
	txBuffer[0] = 0x8f;
	txBuffer[1] = LOR_TX_BUFFER_START;
	txBuffer[2] = 0;
	LOR_SetCmd(3);
	return LOR_EVENT_OK;
}

static LOR_Event LOR_SetDioIrqParams(void)
{
	txBuffer[0] = 0x8d;
	txBuffer[1] = LOR_IRQMASK_HIGH_RXTXTIMEOUT;
	txBuffer[2] = LOR_IRQMASK_LOW_RXDONE | LOR_IRQMASK_LOW_TXDONE | LOR_IRQMASK_LOW_CRCERROR;
	txBuffer[3] = LOR_IRQMASK_HIGH_RXTXTIMEOUT;
	txBuffer[4] = LOR_IRQMASK_LOW_RXDONE | LOR_IRQMASK_LOW_TXDONE;
	txBuffer[5] = 0;
	txBuffer[6] = 0;
	txBuffer[7] = 0;
	txBuffer[8] = 0;
	LOR_SetCmd(9);
	return LOR_EVENT_OK;
}

static LOR_Event LOR_SetPacketParams(uint8_t payloadLen)
{
	txBuffer[0] = 0x8c;
	txBuffer[1] = (uint8_t)(LOR_PBLE_LEN_EXP << 4) | LOR_PBLE_LEN_MANT;
	txBuffer[2] = LOR_EXPLICIT_HEADER;
	txBuffer[3] = payloadLen;
	txBuffer[4] = LOR_CRC_ENABLE;
	txBuffer[5] = LORA_IQ_INVERTED;
	txBuffer[6] = 0;
	txBuffer[7] = 0;
	LOR_SetCmd(8);
	return LOR_EVENT_OK;
}

static LOR_Event LOR_SetTxParams(void)
{
	txBuffer[0] = 0x8e;
	txBuffer[1] = 0x1f; //13 dbm
	txBuffer[2] = 0xe0; //50 us ramp
	LOR_SetCmd(3);
	return LOR_EVENT_OK;
}

static LOR_Event LOR_SetTx(uint16_t timeout)
{
	txBuffer[0] = 0x83;
	txBuffer[1] = 0x02; //periodBase = 0x02, count unit is ms
	txBuffer[2] = (uint8_t)(timeout >> 8);
	txBuffer[3] = (uint8_t)timeout;
	LOR_SetCmd(4);
	return LOR_EVENT_OK;
}
/* wait to test*/
static LOR_Event LOR_SetSleep(void)
{
    txBuffer[0] = 0x84;
	txBuffer[1] = 0x00;
	LOR_SetCmd(2);
    return LOR_EVENT_OK;
}
/* wait to test*/
static LOR_Event LOR_Wakeup(void)
{
    LOR_RESET_L();
    osDelay(2);
//	HAL_Delay(2);
    LOR_RESET_H();
    return LOR_EVENT_OK;
}
static LOR_Event LOR_SetRx(uint16_t timeout)
{
	txBuffer[0] = 0x82;
	txBuffer[1] = 0x02; //periodBase = 0x02, count unit is ms
	txBuffer[2] = (uint8_t)(timeout >> 8);
	txBuffer[3] = (uint8_t)timeout;
	LOR_SetCmd(4);
	return LOR_EVENT_OK;
}

static LOR_Event LOR_GetIrqStatus(void)
{
	txBuffer[0] = 0x15;
	LOR_GetCmd(4);
	return LOR_EVENT_OK;
}

static LOR_Event LOR_ClearIrqStatus(void)
{
	txBuffer[0] = 0x97;
	txBuffer[1] = 0xff;
	txBuffer[2] = 0xff;
	LOR_SetCmd(3);
	return LOR_EVENT_OK;
}

static LOR_Event LOR_WriteTxBuffer(uint8_t *data, uint32_t size)
{
	uint32_t i;
	uint8_t *p;
	txBuffer[0] = 0x1a;
	txBuffer[1] = LOR_TX_BUFFER_START;
	p = txBuffer + 2;
	for (i = 0; i < size; i++)
	{
		*p = *data;
		p++;
		data++;
	}
	LOR_SetCmd(size + 2);
	return LOR_EVENT_OK;
}

static LOR_Event LOR_ReadRxBuffer(uint8_t *data, uint32_t size)
{
	uint32_t i;
	uint8_t *p;
	txBuffer[0] = 0x1b;
	txBuffer[1] = 0;
	LOR_GetCmd(size + 3);
	p = rxBuffer + 3;
	for (i = 0; i < size; i++)
	{
		*data = *p;
		p++;
		data++;
	}
	return LOR_EVENT_OK;
}

static LOR_Event LOR_GetRxBufferStatus(uint8_t *payLoadLen, uint8_t *offset)
{
	txBuffer[0] = 0x17;
	LOR_GetCmd(4);
	*payLoadLen = rxBuffer[2];
	*offset = rxBuffer[3];
	return LOR_EVENT_OK;
}

LOR_Event LOR_Init(void)
{
    LOR_Semaphore = osSemaphoreNew(1,0, &LOR_Semaphore_Attr);
    if(LOR_Semaphore == NULL)
    {
        PRINT_DBG("LOR semaphore create failed \r\n");
        return LOR_EVENT_ERR;
    }
    else
    {
        //LOR_SetStandbyXTALMode();
        LOR_Wakeup();
        LOR_GetStatus();
        LOR_SetPacketType();
        LOR_SetRfFrequency();
        LOR_SetBufferBaseAddress();
        LOR_SetModulationParams();
        LOR_SetPacketParams(0);
        LOR_SetDioIrqParams();
        LOR_SetTxParams();
        return LOR_EVENT_OK;
    }
}
LOR_Event LOR_DeInit(void)
{
    osStatus_t res;
    res = osSemaphoreDelete(LOR_Semaphore);
    if(res != osOK)
    {
        PRINT_DBG("LOR semaphore delete failed \r\n");
        return LOR_EVENT_ERR;
    }
    else
    {
        LOR_SetSleep();
        return LOR_EVENT_OK;
    }
}
void LOR_Reset(void)
{
    LOR_RESET_L();
	osDelay(2);
//	HAL_Delay(2);
	LOR_RESET_H();
    osDelay(100);
//	HAL_Delay(100);
}
LOR_Event LOR_Receive(uint8_t *data, uint32_t *size, uint16_t timeout)
{
    osStatus_t res;
	uint8_t payLoadLen, offset;
	LOR_SetRx(timeout);
	//osSignalWait(SIGNAL_FLAG_LORA_IRQ, 500);
//		osDelay (50);
    res = osSemaphoreAcquire(LOR_Semaphore,500);
    if(res != osOK)
    {
        PRINT_DBG("LOR receive wait semaphore timeout \r\n");
        return LOR_EVENT_ERR;
    }
    else
    { 
        LOR_GetIrqStatus();
        LOR_ClearIrqStatus();
        if (rxBuffer[2] & LOR_IRQMASK_HIGH_RXTXTIMEOUT)
        {
            return LOR_EVENT_TIMEOUT;
        }
        else if (rxBuffer[3] & LOR_IRQMASK_LOW_RXDONE)
        {
            if (!(rxBuffer[3] & LOR_IRQMASK_LOW_CRCERROR))
            {
                LOR_GetRxBufferStatus(&payLoadLen, &offset);
                LOR_ReadRxBuffer(data, payLoadLen);
                *size = payLoadLen;
                return LOR_EVENT_OK;
            }
            else
            {
                return LOR_EVENT_ERR;
            }
        }
        else
        {
            return LOR_EVENT_ERR;
        }
			}		
}


LOR_Event LOR_Transmit(uint8_t *data, uint32_t size, uint16_t timeout)
{
	LOR_SetPacketParams(size);
	LOR_WriteTxBuffer(data, size);
	LOR_SetTx(timeout);
	//osSignalWait(SIGNAL_FLAG_LORA_IRQ, timeout);
    osSemaphoreAcquire(LOR_Semaphore,500);
    LOR_GetIrqStatus();
    LOR_ClearIrqStatus();
    if (rxBuffer[2] & LOR_IRQMASK_HIGH_RXTXTIMEOUT)
    {
        PRINT_DBG("lora send timeout \r\n");
        return LOR_EVENT_TIMEOUT;
    }
    else if (rxBuffer[3] & LOR_IRQMASK_LOW_TXDONE)
    {
        //PRINT_DBG("lor ok \r\n");
        return LOR_EVENT_OK;
    }
    else if (rxBuffer[3] & LOR_IRQMASK_SYNC_WORD_ERROR)
    {
        PRINT_DBG("lor sync word error \r\n");
        return LOR_EVENT_ERR;
    }
    else if (rxBuffer[3] & LOR_IRQMASK_HEADER_ERROR)
    {
        PRINT_DBG("lor header error \r\n");
        return LOR_EVENT_ERR;
    }
    else if (rxBuffer[3] & LOR_IRQMASK_CRC_ERROR)
    {
        PRINT_DBG("lor crc error \r\n");
        return LOR_EVENT_ERR;
    }
    return LOR_EVENT_ERR;
}

void LOR_EXTI_Callback(void)
{
	//osSignalSet(LOR_thread_id, SIGNAL_FLAG_LORA_IRQ);
    osSemaphoreRelease(LOR_Semaphore);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
//	LOR_Semaphore = 1;
	
	LOR_EXTI_Callback();
}

//void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
//{
//	if (hspi->Instance == SPI2)
//	{
//		LOR_CS_H;
//		osMutexRelease(spi2_mutex);
//	}
//}
