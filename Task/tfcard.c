#include "tfcard.h"
#include "main.h"




#define BLOCK_SIZE                       512

extern SD_HandleTypeDef hsd;

HAL_SD_CardInfoTypeDef SDCardInfo;


int8_t STORAGE_GetCapacity_FS (uint8_t lun, uint32_t *block_num, uint16_t *block_size)
{
	HAL_SD_GetCardInfo(&hsd, &SDCardInfo);
	*block_num  = SDCardInfo.LogBlockNbr / BLOCK_SIZE;
	*block_size = BLOCK_SIZE;  
	return (0);
}

int8_t STORAGE_Read_FS (uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{  
	HAL_SD_ReadBlocks(&hsd, (uint8_t*)buf, (uint64_t)(blk_addr * BLOCK_SIZE), BLOCK_SIZE, blk_len);
	
	return (0);
}

int8_t STORAGE_Write_FS (uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
	HAL_SD_WriteBlocks(&hsd, (uint8_t*)buf, (uint64_t)(blk_addr * BLOCK_SIZE), BLOCK_SIZE, blk_len);
	return (0);  
}

void TF_Test(void)
{
	printf("%d\r\n",HAL_SD_GetCardInfo(&hsd, &SDCardInfo));
	printf("%d\r\n",SDCardInfo.CardType);
	printf("%d\r\n",SDCardInfo.CardVersion);
	printf("%d\r\n",SDCardInfo.Class);
	printf("%d\r\n",SDCardInfo.RelCardAdd);
	printf("%d\r\n",SDCardInfo.BlockNbr);
	printf("%d\r\n",SDCardInfo.BlockSize);
	printf("%d\r\n",SDCardInfo.LogBlockNbr);
	printf("%d\r\n",SDCardInfo.LogBlockSize);
	

}
