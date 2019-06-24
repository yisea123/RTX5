#include "dw1000.h"
#include "debug.h"
#include "config.h"
#include "stm32f4xx_hal.h"
#include <string.h>
#define DEC_CS_L() HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET)
#define DEC_CS_H() HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET)
#define DEC_WAKEUP_L() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET)
#define DEC_WAKEUP_H() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET)
//DW1000 RESET PIN is special ,see function DEC_HardwareReset() below
#define ANTENNA_DELAY_TX_HIGH 0x40 
#define ANTENNA_DELAY_TX_LOW 0x6d
#define ANTENNA_DELAY_RX_HIGH 0x40
#define ANTENNA_DELAY_RX_LOW 0x6d

// #define ANTENNA_DELAY_TX_HIGH 0x40 
// #define ANTENNA_DELAY_TX_LOW 0x60
// #define ANTENNA_DELAY_RX_HIGH 0x40
// #define ANTENNA_DELAY_RX_LOW 0x60

#define RX_TIMEOUT 0x0900
#define TRIM 14
const osSemaphoreAttr_t DEC_Semaphore_Attr = {.name = "DEC_Semaphore"};
typedef enum
{
    DEC_REG_DEV_ID = 0x00,     //4  		RO 	  	 Device Identifier ?C includes device type and revision info
    DEC_REG_EUI = 0x01,        //8  		RW 	  	 Extended Unique Identifier
    DEC_REG_PANADR = 0x03,     //4  		RW 	  	 PAN Identifier and Short Address
    DEC_REG_SYS_CFG = 0x04,    //4  		RW 	  	 System Configuration bitmap
    DEC_REG_SYS_TIME = 0x06,   //5 	 	RO 	  	 System Time Counter (40-bit)
    DEC_REG_TX_FCTRL = 0x08,   //5  		RW 	  	 Transmit Frame Control
    DEC_REG_TX_BUFFER = 0x09,  //1024  RW 	  	 Transmit Data Buffer
    DEC_REG_DX_TIME = 0x0A,    //5  		RW 	  	 Delayed Send or Receive Time (40-bit)
    DEC_REG_RX_FWTO = 0x0C,    //2  		RW 	  	 Receive Frame Wait Timeout Period
    DEC_REG_SYS_CTRL = 0x0D,   //4  		SRW  	 	 System Control Register
    DEC_REG_SYS_MASK = 0x0E,   //4			RW 	  	 System Event Mask Register
    DEC_REG_SYS_STATUS = 0x0F, //5		  SRW   	 System Event Status Register
    DEC_REG_RX_FINFO = 0x10,   //4 	  ROD  	 	 RX Frame Information  (in double buffer set)
    DEC_REG_RX_BUFFER = 0x11,  //1024  RWD	  	 Receive Data Buffer  (in double buffer set)
    DEC_REG_RX_FQUAL = 0x12,   //8 	  ROD	  	 Rx Frame Quality information (in double buffer set)
    DEC_REG_RX_TTCKI = 0x13,   //4 	  ROD   	 Receiver Time Tracking Interval  (in double buffer set)

    DEC_REG_RX_TTCKO = 0x14, //5 	  ROD    	 Receiver Time Tracking Offset  (in double buffer set)
    DEC_REG_RX_TIME = 0x15,  //14	  ROD 	   Receive Message Time of Arrival  (in double buffer set)
    DEC_REG_TX_TIME = 0x17,  //10 	  RO 	  	 Transmit Message Time of Sending
    DEC_REG_TX_ANTD = 0x18,  //2 	  RW 	  	 16-bit Delay from Transmit to Antenna

    DEC_REG_SYS_STATE = 0x19,  //5 	  RO 	  	 System State information
    DEC_REG_ACK_RESP_T = 0x1A, //4 	  RW 	  	 Acknowledgement Time and Response Time
    DEC_REG_RX_SNIFF = 0x1D,   //4	 	  RW 	  	 Pulsed Preamble Reception Configuration
    DEC_REG_TX_POWER = 0x1E,   //4		  RW 	  	 TX Power Control
    DEC_REG_CHAN_CTRL = 0x1F,  //4  		RW 	  	 Channel Control
    DEC_REG_USR_SFD = 0x21,    //41 	  RW 	  	 User-specified short/long TX/RX SFD sequences
    DEC_REG_AGC_CTRL = 0x23,   //32	  RW 	  	 Automatic Gain Control configuration
    DEC_REG_EXT_SYNC = 0x24,   //12 	  RW 	  	 External synchronisation control.
    DEC_REG_ACC_MEM = 0x25,    //4064  RO 	  	 Read access to accumulator data
    DEC_REG_GPIO_CTRL = 0x26,  //44 	  RW 	  	 Peripheral register bus 1 access - GPIO control
    DEC_REG_DRX_CONF = 0x27,   //44 	  RW 	  	 Digital Receiver configuration
    DEC_REG_RF_CONF = 0x28,    //58 	  RW	  	 Analog RF Configuration
    DEC_REG_TX_CAL = 0x2A,     //52 	  RW 	  	 Transmitter calibration block
    DEC_REG_FS_CTRL = 0x2B,    //21	  RW 	  	 Frequency synthesiser control block
    DEC_REG_AON = 0x2C,        //12 	  RW 	  	 Always-On register set
    DEC_REG_OTP_IF = 0x2D,     //18 	  RW 			 One Time Programmable Memory Interface
    DEC_REG_LDE_CTRL = 0x2E,   //- 	  RW	  	 Leading edge detection control block
    DEC_REG_DIG_DIAG = 0x2F,   //41	  RW 	 		 Digital Diagnostics Interface
    DEC_REG_PMSC = 0x36,       //48 	  RW Power Management System Control Block
} DEC_Reg;

typedef enum
{
    DEC_DATA_RATE_110 = 0,
    DEC_DATA_RATE_850 = 1,
    DEC_DATA_RATE_6800 = 2
} DEC_DataRate;
typedef enum
{
    DEC_PRF_16 = 1,
    DEC_PRF_64 = 2
} DEC_PRF;
typedef enum
{
    DEC_PREAM_LEN_64 = 0,
    DEC_PREAM_LEN_128 = 1,
    DEC_PREAM_LEN_256 = 2,
    DEC_PREAM_LEN_512 = 3,
    DEC_PREAM_LEN_1024 = 4,
    DEC_PREAM_LEN_1536 = 5,
    DEC_PREAM_LEN_2048 = 6,
    DEC_PREAM_LEN_4096 = 7
} DEC_PreambleLen;
typedef enum
{
    DEC_CHANNEL_1,
    DEC_CHANNEL_2,
    DEC_CHANNEL_3,
    DEC_CHANNEL_4,
    DEC_CHANNEL_5,
    DEC_CHANNEL_7
} DEC_Channel;

extern SPI_HandleTypeDef hspi1;

static uint8_t txBaseBuffer[32];
static uint8_t rxBaseBuffer[34];
static uint8_t *txBuffer;
static uint8_t *rxBuffer;
//static osThreadId DEC_thread_id;
osSemaphoreId_t DEC_SemaphoreId;

static DEC_Event DEC_SPI_Read(DEC_Reg regAddress, uint32_t subReg, uint32_t size);
static DEC_Event DEC_SPI_Write(DEC_Reg regAddress, uint32_t subReg, uint32_t size);
static DEC_Event DEC_GetDeviceId(void);
static DEC_Event DEC_Reset(void);
static DEC_Event DEC_SetPower(uint32_t da, uint32_t mixer);
static DEC_Event DEC_ConfigPacket(DEC_DataRate rate, DEC_PRF prf, DEC_PreambleLen preamLen, DEC_Channel channel, uint32_t frameLen);
static DEC_Event DEC_HardwareInit(DEC_Channel channel);
static DEC_Event DEC_ClearIrq(void);
static DEC_Event DEC_SetRxTimeout(uint32_t timeout);
static DEC_Event DEC_SetTrim(uint32_t trim);
static DEC_Event DEC_SPIBoost(void);
static DEC_Event DEC_GetSystemStatus(uint32_t *systemStatus);
static DEC_Event DEC_HardwareReset(void);
DEC_Event DEC_GetRxTime(uint8_t *timeStamp)
{
    uint32_t i;
    DEC_SPI_Read(DEC_REG_RX_TIME, 0, 5);
    for (i = 0; i < 5; i++)
    {
        timeStamp[i] = rxBuffer[i];
    }
    return DEC_EVENT_OK;
}

DEC_Event DEC_GetTxTime(uint8_t *timeStamp)
{
    uint32_t i;
    DEC_SPI_Read(DEC_REG_TX_TIME, 0, 5);
    for (i = 0; i < 5; i++)
    {
        timeStamp[i] = rxBuffer[i];
    }
    return DEC_EVENT_OK;
}

void DEC_EXTI_Callback(void)
{
    osSemaphoreRelease(DEC_SemaphoreId); 
}

static DEC_Event DEC_GetSystemStatus(uint32_t *systemStatus)
{
    DEC_SPI_Read(DEC_REG_SYS_STATUS, 0, 4);
    *systemStatus = *((uint32_t *)(rxBuffer));
    return DEC_EVENT_OK;
}

DEC_Event DEC_Receive(uint8_t *timeStamp)
{
    uint32_t i;
    //osEvent event;
    osStatus_t res;
    if (timeStamp != NULL)
    {
        for (i = 0; i < 5; i++)
        {
            txBuffer[i] = timeStamp[i];
        }
        DEC_SPI_Write(DEC_REG_DX_TIME, 0, 5);
        txBuffer[0] = 0x03;
    }
    else
    {
        txBuffer[0] = 0x01;
    }
    DEC_SPI_Write(DEC_REG_SYS_CTRL, 0x01, 1);
    res = osSemaphoreAcquire(DEC_SemaphoreId, 100);
    DEC_GetSystemStatus(&i);
    DEC_ClearIrq();
    if ((i & 0xff00) == 0x6f00) //receive done
    {
        //PRINT_DBG("deca rec success \r\n");
        return DEC_EVENT_OK;
    }
    else if ((i & 0x0ff000) == 0x020000)
    {
        return DEC_EVENT_TIMEOUT;
    }
    else if(i & 0x08000000u)
    {
        DEC_TRXCancel();
        return DEC_EVENT_TIMEOUT;
    }
    else
    {
        return DEC_EVENT_ERR;
    }
}

DEC_Event DEC_Transmit(uint8_t *timeStamp)
{
    uint32_t i;
    osStatus_t res;
    timeStamp[0] = 0;
    timeStamp[1] &= 0xfe;
    if (timeStamp != NULL)
    {
        for (i = 0; i < 5; i++)
        {
            txBuffer[i] = timeStamp[i];
        }
        DEC_SPI_Write(DEC_REG_DX_TIME, 0, 5);
        txBuffer[0] = 0x06;
    }
    else
    {
        txBuffer[0] = 0x02;
    }
    DEC_SPI_Write(DEC_REG_SYS_CTRL, 0, 1);
    //osSignalWait(SIGNAL_FLAG_DECA_IRQ, osWaitForever);
    res = osSemaphoreAcquire(DEC_SemaphoreId, 100);
    if(res != osOK)
    {
        PRINT_DBG("DEC semaphore aquire failed \r\n");
    }
    DEC_GetSystemStatus(&i);
    DEC_ClearIrq();
    
    if ((i & 0xff) == 0xf3) //transmit done
    {
        return DEC_EVENT_OK;
    }
    else
    {
        return DEC_EVENT_ERR;
    }
}

static DEC_Event DEC_SPIBoost(void)
{
    HAL_SPI_DeInit(&hspi1);
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
    HAL_SPI_Init(&hspi1);
    return DEC_EVENT_OK;
}
static DEC_Event DEC_SPIReset(void)
{
    HAL_SPI_DeInit(&hspi1);
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
    HAL_SPI_Init(&hspi1);
    return DEC_EVENT_OK;
}
static DEC_Event DEC_SetTrim(uint32_t trim)
{
    txBuffer[0] = trim | 0x60;
    DEC_SPI_Write(DEC_REG_FS_CTRL, 0x0e, 1);
    return DEC_EVENT_OK;
}

static DEC_Event DEC_SetRxTimeout(uint32_t timeout)
{
    txBuffer[0] = (uint8_t)timeout;
    txBuffer[1] = (uint8_t)((timeout >> 8));
    DEC_SPI_Write(DEC_REG_RX_FWTO, 0, 2);
    return DEC_EVENT_OK;
}

static DEC_Event DEC_ClearIrq(void)
{
    txBuffer[0] = 0x80;
    txBuffer[1] = 0x30;
    txBuffer[2] = 0x03;
    txBuffer[3] = 0x0c;
    DEC_SPI_Write(DEC_REG_SYS_STATUS, 0, 4);
    return DEC_EVENT_OK;
}

static DEC_Event DEC_SPI_Write(DEC_Reg regAddress, uint32_t subReg, uint32_t size)
{
    uint32_t offset = 2;

    if (subReg != 0)
    {
        regAddress |= 0x40;
        if ((subReg >> 7) == 0) //single byte subReg
        {
            txBaseBuffer[2] = subReg;
            offset = 1;
        }
        else //double byte subReg
        {
            txBaseBuffer[1] = (subReg & 0x7f) | 0x80;
            txBaseBuffer[2] = subReg >> 7;
            offset = 0;
        }
    }
    txBaseBuffer[offset] = regAddress | 0x80;
    size += 3 - offset;
    DEC_CS_L();
    HAL_SPI_Transmit(&hspi1, txBaseBuffer + offset, size, HAL_MAX_DELAY);
    DEC_CS_H();
    return DEC_EVENT_OK;
}

static DEC_Event DEC_SPI_Read(DEC_Reg regAddress, uint32_t subReg, uint32_t size)
{
    uint32_t offset = 2;

    if (subReg != 0)
    {
        regAddress |= 0x40;
        if ((subReg >> 7) == 0) //single byte subReg
        {
            txBaseBuffer[2] = subReg;
            offset = 1;
        }
        else //double byte subReg
        {
            txBaseBuffer[1] = (subReg & 0x7f) | 0x80;
            txBaseBuffer[2] = subReg >> 7;
            offset = 0;
        }
    }
    txBaseBuffer[offset] = regAddress;
    size += 3 - offset;
    DEC_CS_L();
    HAL_SPI_TransmitReceive(&hspi1, txBaseBuffer + offset, rxBaseBuffer + offset, size, HAL_MAX_DELAY);
    DEC_CS_H();
    return DEC_EVENT_OK;
}

static DEC_Event DEC_GetDeviceId(void)
{
    DEC_SPI_Read(DEC_REG_DEV_ID, 0, 4);
    PRINT_DBG("device ID = %x %x %x %x \r\n", rxBuffer[0], rxBuffer[1], rxBuffer[2], rxBuffer[3]);
    rxBuffer[0] = 0;
    rxBuffer[1] = 0;
    rxBuffer[2] = 0;
    rxBuffer[3] = 0;
    return DEC_EVENT_OK;
}
DEC_Event DEC_GetSignalPower(uint16_t signal_power)
{
    DEC_SPI_Read(DEC_REG_RX_FQUAL, 0x06, 2);
    signal_power = *((uint16_t *)rxBuffer);
    return DEC_EVENT_OK;
}
DEC_Event DEC_Sleep(void)
{
    txBuffer[0] = 0x40; //
    txBuffer[1] = 0x08; //
    DEC_SPI_Write(DEC_REG_AON, 0x00, 2);
    txBuffer[0] = 0x00; //
    DEC_SPI_Write(DEC_REG_AON, 0x0a, 1);
    txBuffer[0] = 0x03; //set sleep enable and setup wakeup pin
    DEC_SPI_Write(DEC_REG_AON, 0x06, 1);
    txBuffer[0] = 0x06; //upload user configuration to AON register
    DEC_SPI_Write(DEC_REG_AON, 0x02, 1);
    return DEC_EVENT_OK;
}
DEC_Event DEC_Wakeup(void)
{
    DEC_WAKEUP_H();
    osDelay(10);
    DEC_WAKEUP_L();
    return DEC_EVENT_OK;
}

DEC_Event DEC_Init(void)
{
    txBuffer = (uint8_t *)(&(txBaseBuffer[0])) + 3;
    rxBuffer = (uint8_t *)(&(rxBaseBuffer[0])) + 3;
    //DEC_thread_id = osThreadGetId();
	//DEC_Wakeup();
	//DEC_HardwareReset();
	for(;;)
	{
		DEC_GetDeviceId();
		osDelay(500);
	}
//    DEC_SemaphoreId = osSemaphoreNew(1,0, &DEC_Semaphore_Attr);
//    if(DEC_SemaphoreId == NULL)
//    {
//        PRINT_DBG("DEC semaphore create failed \r\n");
//        return DEC_EVENT_ERR;
//    }
//    else
//    { 
//        DEC_Wakeup();
//        DEC_HardwareReset();
//        DEC_SetPower(DEC_POWER_DA, DEC_POWER_MIXER);
//        DEC_ConfigPacket(DEC_DATA_RATE_850, DEC_PRF_64, DEC_PREAM_LEN_256, DECAWAVE_CHANNEL, DEC_FRAME_LEN);
//        DEC_HardwareInit(DECAWAVE_CHANNEL);
//        DEC_ClearIrq();
//        DEC_SetRxTimeout(RX_TIMEOUT);
//        DEC_SetTrim(TRIM);
//        DEC_GetDeviceId();
//        DEC_SPIBoost();
//        return DEC_EVENT_OK;
//    }
}
DEC_Event DEC_DeInit(void)
{
    osStatus_t res;
    res = osSemaphoreDelete(DEC_SemaphoreId);
    if(res != osOK)
    {
        PRINT_DBG("DEC semaphore delete failed \r\n");
        return DEC_EVENT_ERR;
    }
    else
    {
        DEC_HardwareReset();
        DEC_Sleep();
        DEC_SPIReset();
        return DEC_EVENT_OK;
    }
}
DEC_Event DEC_Reset(void)
{
    txBuffer[0] = 0x01;
    DEC_SPI_Write(DEC_REG_PMSC, 0, 1);
    txBuffer[0] = 0x00;
    DEC_SPI_Write(DEC_REG_PMSC, 0x03, 1);
    txBuffer[0] = 0xf0;
    DEC_SPI_Write(DEC_REG_PMSC, 0x03, 1);
    return DEC_EVENT_OK;
}
/*
    DW1000 reset 
*/
static DEC_Event DEC_HardwareReset(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    osDelay(10);
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    osDelay(10);
    return DEC_EVENT_OK;
}
static DEC_Event DEC_SetPower(uint32_t da, uint32_t mixer)
{
    mixer |= (da << 4);
    txBuffer[0] = mixer;
    txBuffer[1] = mixer;
    txBuffer[2] = mixer;
    txBuffer[3] = mixer;
    DEC_SPI_Write(DEC_REG_TX_POWER, 0, 4);
    return DEC_EVENT_OK;
}

static DEC_Event DEC_ConfigPacket(DEC_DataRate rate, DEC_PRF prf, DEC_PreambleLen preamLen, DEC_Channel channel, uint32_t frameLen)
{
    uint8_t a;

    /*bits:14,13   00:110kbps   01:850kbps  10:6.8Mbps  11:reserved
  */
    txBuffer[0] = frameLen;           //00001010
    txBuffer[1] = (rate << 5) | 0x80; //10100000
    txBuffer[2] = prf;
    a = preamLen + 4;
    if (a == 11)
        a = 12; //calculate preamble length code
    txBuffer[2] |= a & 0x0c;
    txBuffer[2] |= (a & 0x03) << 4;        //00110110
    DEC_SPI_Write(DEC_REG_TX_FCTRL, 0, 3); //TX_FCTRL transmit frame control

    switch (rate)
    {
        /*�Ĵ���0x27�����ֽ��ջ�����
    */
    case DEC_DATA_RATE_110:
        txBuffer[0] = 0x0a;
        txBuffer[1] = 0x00;
        DEC_SPI_Write(DEC_REG_DRX_CONF, 2, 2); //set DRX_TUNE0b
        /*�Ĵ���0x04��ϵͳ����
    */
        txBuffer[0] = 0x44;                   //no smart power, low data rate
        txBuffer[1] = 0x10;                   //enable receiver timeout
        DEC_SPI_Write(DEC_REG_SYS_CFG, 2, 2); //SYS_CFG
        break;
    case DEC_DATA_RATE_850:
        txBuffer[0] = 0x01;
        txBuffer[1] = 0x00;
        DEC_SPI_Write(DEC_REG_DRX_CONF, 2, 2);
        txBuffer[0] = 0x04;
        txBuffer[1] = 0x10;
        DEC_SPI_Write(DEC_REG_SYS_CFG, 2, 2);
        break;
    case DEC_DATA_RATE_6800:
        txBuffer[0] = 0x01;
        txBuffer[1] = 0x00;
        DEC_SPI_Write(DEC_REG_DRX_CONF, 2, 2);
        txBuffer[0] = 0x00;
        txBuffer[1] = 0x10;
        DEC_SPI_Write(DEC_REG_SYS_CFG, 2, 2);
        break;
    }

    if (prf == DEC_PRF_16) //16 MHz PRF
    {
        txBuffer[0] = 0x87;
        txBuffer[1] = 0x00;
        DEC_SPI_Write(DEC_REG_DRX_CONF, 4, 2); //set DRX_TUNE1a
        /*�Ĵ���0x23���Զ��������?
    */
        txBuffer[0] = 0x70;
        txBuffer[1] = 0x88;
        DEC_SPI_Write(DEC_REG_AGC_CTRL, 4, 2); //set AGC_TUNE1
        txBuffer[1] = 0x07;
        txBuffer[2] = 0x06;
        DEC_SPI_Write(DEC_REG_LDE_CTRL, 0x1806, 2); //LDE_CFG2

        switch (channel)
        {

        case DEC_CHANNEL_1:
            txBuffer[0] = 0x11;
            txBuffer[1] = 0x00;
            txBuffer[2] = 0x84; //PRF
            txBuffer[3] = 0x10; //code 2
            break;
        case DEC_CHANNEL_2:
            txBuffer[0] = 0x22;
            txBuffer[1] = 0x00;
            txBuffer[2] = 0x04; //PRF
            txBuffer[3] = 0x21; //code 4
            break;
        case DEC_CHANNEL_3:
            txBuffer[0] = 0x33;
            txBuffer[1] = 0x00;
            txBuffer[2] = 0x84; //PRF
            txBuffer[3] = 0x31; //code 6
            break;
        case DEC_CHANNEL_4:
            txBuffer[0] = 0x44;
            txBuffer[1] = 0x00;
            txBuffer[2] = 0x04; //PRF
            txBuffer[3] = 0x42; //code 8
            break;
        case DEC_CHANNEL_5:
            txBuffer[0] = 0x55;
            txBuffer[1] = 0x00;
            txBuffer[2] = 0x04; //PRF
            txBuffer[3] = 0x21; //code 4
            break;
        case DEC_CHANNEL_7:
            txBuffer[0] = 0x77;
            txBuffer[1] = 0x00;
            txBuffer[2] = 0x04; //PRF
            txBuffer[3] = 0x42; //code 8
            break;
        }
        DEC_SPI_Write(DEC_REG_CHAN_CTRL, 0, 4); //CHAN_CTRL default channel, code etc

        if (preamLen < 2) //decide PAC size
        {
            txBuffer[0] = 0x2d;
            txBuffer[1] = 0x00;
            txBuffer[2] = 0x1a;
            txBuffer[3] = 0x31;
        }
        else if (preamLen < 4)
        {
            txBuffer[0] = 0x52;
            txBuffer[1] = 0x00;
            txBuffer[2] = 0x1a;
            txBuffer[3] = 0x33;
        }
        else if (preamLen < 5)
        {
            txBuffer[0] = 0x9a;
            txBuffer[1] = 0x00;
            txBuffer[2] = 0x1a;
            txBuffer[3] = 0x35;
        }
        else
        {
            txBuffer[0] = 0x1d;
            txBuffer[1] = 0x01;
            txBuffer[2] = 0x1a;
            txBuffer[3] = 0x37;
        }
        DEC_SPI_Write(DEC_REG_DRX_CONF, 8, 4); //set DRX_TUNE2 (PAC size too)
    }
    else //64 MHz PRF
    {
        txBuffer[0] = 0x8d;
        txBuffer[1] = 0x00;
        DEC_SPI_Write(DEC_REG_DRX_CONF, 4, 2); //set DRX_TUNE1a
        /*�Ĵ���0x23���Զ��������?
    */
        txBuffer[0] = 0x9b;
        txBuffer[1] = 0x88;
        DEC_SPI_Write(DEC_REG_AGC_CTRL, 4, 2); //set AGC_TUNE1
        txBuffer[0] = 0x07;
        txBuffer[1] = 0x06;
        DEC_SPI_Write(DEC_REG_LDE_CTRL, 0x1806, 2); //LDE_CFG2

        switch (channel)
        {

        case DEC_CHANNEL_1:
            txBuffer[0] = 0x11;
            txBuffer[1] = 0x00;
            txBuffer[2] = 0x08; //PRF
            txBuffer[3] = 0x63; //code 12
            break;
        case DEC_CHANNEL_2:
            txBuffer[0] = 0x22;
            txBuffer[1] = 0x00;
            txBuffer[2] = 0x88; //PRF
            txBuffer[3] = 0x52; //code 12
            break;
        case DEC_CHANNEL_3:
            txBuffer[0] = 0x33;
            txBuffer[1] = 0x00;
            txBuffer[2] = 0x08; //PRF
            txBuffer[3] = 0x63; //code 12
            break;
        case DEC_CHANNEL_4:
            txBuffer[0] = 0x44;
            txBuffer[1] = 0x00;
            txBuffer[2] = 0x08; //PRF
            txBuffer[3] = 0xa5; //code 12
            break;
        case DEC_CHANNEL_5:
            txBuffer[0] = 0x55;
            txBuffer[1] = 0x00;
            txBuffer[2] = 0x08; //PRF
            txBuffer[3] = 0x63; //code 20
            break;
        case DEC_CHANNEL_7:
            txBuffer[0] = 0x77;
            txBuffer[1] = 0x00;
            txBuffer[2] = 0x08; //PRF
            txBuffer[3] = 0xa5; //code 12
            break;
        }
        DEC_SPI_Write(DEC_REG_CHAN_CTRL, 0, 4); //CHAN_CTRL default channel, code etc

        if (preamLen < 2) //decide PAC size
        {
            txBuffer[0] = 0x6b;
            txBuffer[1] = 0x00;
            txBuffer[2] = 0x3b;
            txBuffer[3] = 0x31;
        }
        else if (preamLen < 4)
        {
            txBuffer[0] = 0xbe;
            txBuffer[1] = 0x00;
            txBuffer[2] = 0x3b;
            txBuffer[3] = 0x33;
        }
        else if (preamLen < 5)
        {
            txBuffer[0] = 0x5e;
            txBuffer[1] = 0x01;
            txBuffer[2] = 0x3b;
            txBuffer[3] = 0x35;
        }
        else
        {
            txBuffer[0] = 0x96;
            txBuffer[1] = 0x02;
            txBuffer[2] = 0x3b;
            txBuffer[3] = 0x37;
        }
        DEC_SPI_Write(DEC_REG_DRX_CONF, 8, 4); //set DRX_TUNE2 (PAC size too)
    }

    if (preamLen > 4) //preamble length longer than 1024
    {
        txBuffer[0] = 0x64;
        txBuffer[1] = 0x00;
    }
    else if (preamLen > 0) //preamble length between 128 and 1024
    {
        txBuffer[0] = 0x20;
        txBuffer[1] = 0x00;
    }
    else //preamble length 64
    {
        txBuffer[0] = 0x10;
        txBuffer[1] = 0x00;
    }
    DEC_SPI_Write(DEC_REG_DRX_CONF, 6, 2);
    return DEC_EVENT_OK;
}

static DEC_Event DEC_HardwareInit(DEC_Channel channel)
{
    /*�Ĵ���0x23:�Զ������������?
  Sub-Register 0x23:0C ��32 bit�ĵ�г�Ĵ���
  ֻ��д��0x2502A907,д������ֵ���ܻ���ֹ���?
  */
    txBuffer[0] = 0x07;
    txBuffer[1] = 0xA9;
    txBuffer[2] = 0x02;
    txBuffer[3] = 0x25;
    DEC_SPI_Write(DEC_REG_AGC_CTRL, 0x0c, 4); //set AGC_TUNE2

    /*�Ĵ���0x2e:ǰ�ؼ��ӿ� ����������ۼ������ݣ�?
  Sub-Register 0x2E:0806��LDE���üĴ���1
  */
    txBuffer[0] = 0x0D;
    DEC_SPI_Write(DEC_REG_LDE_CTRL, 0x0806, 1); //set LDE_CFG1

    switch (channel)
    {
        /*�Ĵ���0x28:ģ��RF������
    Sub-Register 0x28:0C�C RF_TXCTRL
    Sub-Register 0x28:0B�C RF_RXCTRLH
    */

        /*�Ĵ���0x2a:����У׼��
    Sub-Register 0x2A:0B �C TC_PGDELAY ���������巢�����ӳ�ֵ
    */

        /*�Ĵ���0x2b:Ƶ�ۿ���������Ҫ��������channel�����Ƶ������?
    Sub-Register 0x2B:07 �C FS_PLLCFG �����໷���üĴ�����
    Sub-Register 0x2B:0B �C FS_PLLTUNE�����໷��г�Ĵ�����
    */
    case DEC_CHANNEL_1:
        txBuffer[0] = 0x40;
        txBuffer[1] = 0x5C;
        txBuffer[2] = 0x00;
        txBuffer[3] = 0x00;
        DEC_SPI_Write(DEC_REG_RF_CONF, 0x0c, 4); //RF_TXCTRL for channel 1

        txBuffer[0] = 0xd8;
        DEC_SPI_Write(DEC_REG_RF_CONF, 0x0b, 1); //RF_RXCTRLH for channel 1

        txBuffer[0] = 0xC9;
        DEC_SPI_Write(DEC_REG_TX_CAL, 0x0b, 1); //TC_PGDELAY puslse width 1

        txBuffer[0] = 0x07;
        txBuffer[1] = 0x04;
        txBuffer[2] = 0x00;
        txBuffer[3] = 0x09;
        DEC_SPI_Write(DEC_REG_FS_CTRL, 0x07, 4); //FS_PLLCFG

        txBuffer[0] = 0x1e;
        DEC_SPI_Write(DEC_REG_FS_CTRL, 0x0b, 1); //FS_PLLTUNE
        break;

    case DEC_CHANNEL_2:
        txBuffer[0] = 0xa0;
        txBuffer[1] = 0x5C;
        txBuffer[2] = 0x04;
        txBuffer[3] = 0x00;
        DEC_SPI_Write(DEC_REG_RF_CONF, 0x0c, 4);

        txBuffer[0] = 0xd8;
        DEC_SPI_Write(DEC_REG_RF_CONF, 0x0b, 1);

        txBuffer[0] = 0xC2;
        DEC_SPI_Write(DEC_REG_TX_CAL, 0x0b, 1);

        txBuffer[0] = 0x08;
        txBuffer[1] = 0x05;
        txBuffer[2] = 0x40;
        txBuffer[3] = 0x08;
        DEC_SPI_Write(DEC_REG_FS_CTRL, 0x07, 4);

        txBuffer[0] = 0x26;
        DEC_SPI_Write(DEC_REG_FS_CTRL, 0x0b, 1);
        break;
    case DEC_CHANNEL_3:
        txBuffer[0] = 0xc0;
        txBuffer[1] = 0x6c;
        txBuffer[2] = 0x08;
        txBuffer[3] = 0x00;
        DEC_SPI_Write(DEC_REG_RF_CONF, 0x0c, 4);

        txBuffer[0] = 0xd8;
        DEC_SPI_Write(DEC_REG_RF_CONF, 0x0b, 1);

        txBuffer[0] = 0xC5;
        DEC_SPI_Write(DEC_REG_TX_CAL, 0x0b, 1);

        txBuffer[0] = 0x09;
        txBuffer[1] = 0x10;
        txBuffer[2] = 0x40;
        txBuffer[3] = 0x08;
        DEC_SPI_Write(DEC_REG_FS_CTRL, 0x07, 4);

        txBuffer[0] = 0x5e;
        DEC_SPI_Write(DEC_REG_FS_CTRL, 0x0b, 1);
        break;
    case DEC_CHANNEL_4:
        txBuffer[0] = 0x80;
        txBuffer[1] = 0x5C;
        txBuffer[2] = 0x04;
        txBuffer[3] = 0x00;
        DEC_SPI_Write(DEC_REG_RF_CONF, 0x0c, 4);

        txBuffer[0] = 0xbc;
        DEC_SPI_Write(DEC_REG_RF_CONF, 0x0b, 1);

        txBuffer[0] = 0x95;
        DEC_SPI_Write(DEC_REG_TX_CAL, 0x0b, 1);

        txBuffer[0] = 0x08;
        txBuffer[1] = 0x05;
        txBuffer[2] = 0x40;
        txBuffer[3] = 0x08;
        DEC_SPI_Write(DEC_REG_FS_CTRL, 0x07, 4);

        txBuffer[0] = 0x26;
        DEC_SPI_Write(DEC_REG_FS_CTRL, 0x0b, 1);
        break;
    case DEC_CHANNEL_5:
        txBuffer[0] = 0xe0;
        txBuffer[1] = 0x3f;
        txBuffer[2] = 0x1e;
        txBuffer[3] = 0x00;
        DEC_SPI_Write(DEC_REG_RF_CONF, 0x0c, 4);

        txBuffer[0] = 0xbe;
        DEC_SPI_Write(DEC_REG_RF_CONF, 0x0b, 1);

        txBuffer[0] = 0xC0;
        DEC_SPI_Write(DEC_REG_TX_CAL, 0x0b, 1);

        txBuffer[0] = 0x1d;
        txBuffer[1] = 0x04;
        txBuffer[2] = 0x00;
        txBuffer[3] = 0x08;
        DEC_SPI_Write(DEC_REG_FS_CTRL, 0x07, 4);

        txBuffer[0] = 0xbe;
        DEC_SPI_Write(DEC_REG_FS_CTRL, 0x0b, 1);
        break;
    case DEC_CHANNEL_7:
        txBuffer[0] = 0xe0;
        txBuffer[1] = 0x7d;
        txBuffer[2] = 0x1e;
        txBuffer[3] = 0x00;
        DEC_SPI_Write(DEC_REG_RF_CONF, 0x0c, 4);

        txBuffer[0] = 0xbc;
        DEC_SPI_Write(DEC_REG_RF_CONF, 0x0b, 1);

        txBuffer[0] = 0x93;
        DEC_SPI_Write(DEC_REG_TX_CAL, 0x0b, 1);

        txBuffer[0] = 0x1d;
        txBuffer[1] = 0x04;
        txBuffer[2] = 0x00;
        txBuffer[3] = 0x08;
        DEC_SPI_Write(DEC_REG_FS_CTRL, 0x07, 4);

        txBuffer[0] = 0xbe;
        DEC_SPI_Write(DEC_REG_FS_CTRL, 0x0b, 1);
        break;
    }

    /*�Ĵ���0x36����Դ����Ϳ���?
  SYSCLKS��01��ϵͳʱ��Ϊ19.2MHz XTIʱ�ӣ�
  */
    txBuffer[0] = 0x01;
    txBuffer[1] = 0x03;
    DEC_SPI_Write(DEC_REG_PMSC, 0, 2);

    /*�Ĵ���0x2D��OTP�洢���ӿ�
  Sub-Register 0x2D:06 �C OTP_CTRL
  */
    txBuffer[0] = 0x00;
    txBuffer[1] = 0x80;
    DEC_SPI_Write(DEC_REG_OTP_IF, 0x06, 2); //load LDE algorithm
    osDelay(10);

    /*�Ĵ���0x36����Դ�����ϵͳ����?
  02��ADCCE��Analog-to-Digital Convertor Clock Enable
  */
    txBuffer[0] = 0x00;
    txBuffer[1] = 0x02;
    DEC_SPI_Write(DEC_REG_PMSC, 0, 2);

    /*�Ĵ���0x18�����߷����ӳ�����
  */
    txBuffer[0] = ANTENNA_DELAY_TX_LOW;   //0x60
    txBuffer[1] = ANTENNA_DELAY_TX_HIGH;  //0x40
    DEC_SPI_Write(DEC_REG_TX_ANTD, 0, 2); //TX antenna delay

    /*�Ĵ���0x2e��
  Sub-Register 0x2E:1804 �C LDE_RXANTD�����������ӳ����ã�
  */
    txBuffer[0] = ANTENNA_DELAY_RX_LOW;
    txBuffer[1] = ANTENNA_DELAY_RX_HIGH;
    DEC_SPI_Write(DEC_REG_LDE_CTRL, 0x1804, 2); //RX antenna delay

    /*�Ĵ���0x0E��ϵͳ�¼��Ĵ���
  */
    txBuffer[0] = 0x80;
    txBuffer[1] = 0x30;
    txBuffer[2] = 0x03;
    txBuffer[3] = 0x0c;
    DEC_SPI_Write(DEC_REG_SYS_MASK, 0, 4); //open interrupt

    return DEC_EVENT_OK;
}
DEC_Event DEC_AcquireAnchorMessage(uint8_t *data, uint32_t len)
{
    DEC_SPI_Read(DEC_REG_RX_BUFFER, 0, len);
    memcpy(data, rxBuffer, len);
    return DEC_EVENT_OK;
}
DEC_Event DEC_DecaTimeAdd(uint8_t *start, uint32_t interval, uint8_t *stop)
{
    uint32_t *startTemp, *stopTemp;
    startTemp = (uint32_t *)(start + 1);
    stopTemp = (uint32_t *)(stop + 1);
    *stopTemp = *startTemp + interval;
    return DEC_EVENT_OK;
}

uint32_t DEC_DecaTimeToUint32_t(uint8_t *timeStamp)
{
    uint32_t *temp;
    temp = (uint32_t *)(timeStamp);
    return *temp;
}

DEC_Event DEC_GetSystemTime(uint8_t *currentTime)
{
    DEC_SPI_Read(DEC_REG_SYS_TIME, 0x00, 5);
    memcpy(currentTime, rxBuffer, 5);
    return DEC_EVENT_OK;
}
DEC_Event DEC_TRXCancel(void)
{
    DEC_SPI_Read(DEC_REG_SYS_CTRL, 0x00, 1);
    //txBuffer[0] = rxBuffer[0] | 0x40;  //TRX cancel bit
    txBuffer[0] = 0x40;
    txBuffer[1] = 0x00;
    txBuffer[2] = 0x00;
    txBuffer[3] = 0x00;
    DEC_SPI_Write(DEC_REG_SYS_CTRL, 0x00, 4);
    return DEC_EVENT_OK;
}
