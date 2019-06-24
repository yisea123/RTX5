#include "bq27542.h"

void POWER_OUT(uint8_t m)
{
	switch(m)
	{
		case 0:
			printf("\r\n HAL_OK ");
			break;
		case 1:
			printf("\r\n HAL_ERROR ");
			break;
		case 2:
			printf("\r\n HAL_BUSY ");
			break;
		case 3:
			printf("\r\n HAL_TIMEOUT ");
	}
}
		
  void task_BQ27542_Read(void)
	{
		uint8_t aaa;
		int16_t voltage;
		int16_t averageCurrent;
		aaa=HAL_I2C_Mem_Read(&hi2c1, BQ27542_ADD, 0x08, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&(voltage), 2,1000);POWER_OUT(aaa);
		printf("\r\n Voltage:%d mV\r\n",voltage);	
		aaa=HAL_I2C_Mem_Read(&hi2c1, BQ27542_ADD, 0x14, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&(averageCurrent), 2,1000);POWER_OUT(aaa);
		printf("\r\n Current:%d mA\r\n",averageCurrent);	
		
	}


