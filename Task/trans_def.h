#ifndef __TRANS_DEF_H
#define __TRANS_DEF_H
#include "stm32f4xx_hal.h"
#include "list.h"

//uint64_t utc0_ts_timestamp;
//uint32_t	match_id_flag;

__packed typedef struct	
{
	uint8_t		pkg_begin;
	uint16_t 	content_len;
	uint8_t		op_id;
	uint32_t	central_id;
	uint8_t 	pkg_end;
}INST_REGISTER_CENTRAL_TypeDef;

__packed typedef struct	
{
	uint8_t		pkg_begin;
	uint16_t 	content_len;
	uint8_t		op_id;
	uint64_t	utc0_ts;
//	uint8_t 	anchor_count;
}INST_MEASURE_INFO_SERVER_head_TypeDef;

__packed typedef struct	
{
	uint8_t 	anchor_count;
}INST_MEASURE_INFO_SERVER_anchor_count_TypeDef;

__packed typedef struct	
{
	uint32_t 	anchor_id;
	int16_t		anchor_x;
	int16_t		anchor_y;
	int16_t		anchor_z;
}INST_MEASURE_INFO_SERVER_anchor_TypeDef;

__packed typedef struct	
{
	uint8_t 	anchor_count;
}INST_MEASURE_INFO_SERVER_tag_count_TypeDef;

__packed typedef struct	
{
	uint32_t 	tag_id;
}INST_MEASURE_INFO_SERVER_tag_TypeDef;

__packed typedef struct
{
	uint32_t	match_id;
	uint8_t 	pkg_end;
}INST_MEASURE_INFO_SERVER_tailTypeDef;


__packed typedef struct
{
	uint8_t		pkg_begin;
	uint16_t 	content_len;
	uint8_t		op_id;
	uint8_t		tag_count;
}INST_TAG_DATA_CENTRAL_headTypeDef;

__packed typedef struct
{
	uint32_t	match_id;
	uint8_t 	pkg_end;
}INST_TAG_DATA_CENTRAL_tailTypeDef;

typedef struct 
{
	LIST_NodeTypeDef *p1;

	LIST_NodeTypeDef *p2;
	
	uint8_t Tagnumber ;
}Station_P_Inf;

__packed typedef struct	
{
	uint64_t timestamp;
	uint32_t	match_id;
}INST_DATA_TypeDef;

typedef struct 
{
	uint64_t utc;
	uint32_t id;
	uint8_t batteryPercentage;
	uint32_t packetId;
	uint32_t *reserved;
}LOW_TagProtocolInfoTypeDef;

#endif
