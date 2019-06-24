#ifndef __LORAWAN_DEF_H
#define __LORAWAN_DEF_H
#include "stm32f4xx_hal.h"

__packed typedef struct 
{	
	uint64_t timestamp;
	uint32_t id;
	uint8_t batteryPercentage;	
	int16_t pos_x;
	int16_t pos_y;
	int16_t pos_z;
	int8_t speed_x;
	int8_t speed_y;
	int8_t speed_z;
	int8_t acc_x;
	int8_t acc_y;
	int8_t acc_z;
} TRANS_TagInfoTypeDef;

__packed typedef struct 
{	
	uint64_t timestamp;
	uint32_t id;
	uint8_t batteryPercentage;	
	int16_t pos_x;
	int16_t pos_y;
	int16_t pos_z;
	int8_t speed_x;
	int8_t speed_y;
	int8_t speed_z;
	int8_t acc_x;
	int8_t acc_y;
	int8_t acc_z;
} Tag_data;

__packed typedef struct
{
	uint32_t AnchorId;
//	uint32_t Mask;
//	double Coor[3];
	int16_t Coor_x;
	int16_t Coor_y;
	int16_t Coor_z;
}LOW_AnchorInfoTypeDef;

__packed typedef struct	
{
	uint32_t 	tag_id;
	uint8_t tag_packedid;
}tag_mempool_TypeDef;

#define SamplingInterval  500;

#endif
