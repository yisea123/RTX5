#ifndef __BQ27542_H
#define __BQ27542_H
#endif

#include "stm32f4xx_hal.h"

extern I2C_HandleTypeDef hi2c1;
#define BQ27542_ADD 0xaa


//__packed typedef struct
//{
//	int16_t voltage;
//	int16_t averageCurrent;
//  uint16_t remainingCapacity;
//	uint8_t SOC;
//	uint16_t remainTime;
//}BAT_InfoTypeDef;



void task_BQ27542_Read(void);


