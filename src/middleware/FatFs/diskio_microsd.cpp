#include "ff.h"
#include "diskio.h"
#include "diskio_microsd.h"
#include "peripherals/microsd.hpp"
#include <stdio.h>
#include "system/scheduler.hpp"

DSTATUS MMC_disk_status(void) {
  if (!MicroSD::available()) {
    return STA_NOINIT;
  }
  
  HAL_SD_CardStateTypeDef state = HAL_SD_GetCardState(&MicroSD::hsd);
  if (state != HAL_SD_CARD_TRANSFER) {
    return STA_NOINIT;
  }
  return 0;
}

DSTATUS MMC_disk_initialize(void) {
  if (!MicroSD::available()) {
    return STA_NOINIT;
  }

  // Check if card is already in transfer state
  HAL_SD_CardStateTypeDef state = HAL_SD_GetCardState(&MicroSD::hsd);
  if (state == HAL_SD_CARD_TRANSFER) {
    return 0;
  }

  // Try to reinitialize the card
  if (HAL_SD_Init(&MicroSD::hsd) != HAL_OK) {
    return STA_NOINIT;
  }

  // Check if card is now in transfer state
  state = HAL_SD_GetCardState(&MicroSD::hsd);
  if (state != HAL_SD_CARD_TRANSFER) {
    return STA_NOINIT;
  }

  return 0;
}

DRESULT MMC_disk_read(BYTE* buff, LBA_t sector, UINT count) {
  if (!MicroSD::available()) {
    return RES_NOTRDY;
  }

  HAL_StatusTypeDef status = HAL_SD_ReadBlocks(&MicroSD::hsd, buff, sector, count, 5000);
  if (status == HAL_OK) {
    return RES_OK;
  } else if (status == HAL_TIMEOUT) {
    return RES_NOTRDY;
  } else if (status == HAL_ERROR) {
    return RES_ERROR;
  }
  return RES_ERROR;
}

DRESULT MMC_disk_write(const BYTE* buff, LBA_t sector, UINT count) {
  if (!MicroSD::available()) {
    return RES_NOTRDY;
  }
  SCB_CleanDCache_by_Addr((uint32_t*)buff, count * 512);

  HAL_StatusTypeDef status = HAL_SD_WriteBlocks(&MicroSD::hsd, (uint8_t*)buff, sector, count, 5000);
  if (status == HAL_OK) {
    return RES_OK;
  } else if (status == HAL_TIMEOUT) {
    return RES_NOTRDY;
  } else if (status == HAL_ERROR) {
    return RES_ERROR;
  }
  return RES_ERROR;
}

DRESULT MMC_disk_ioctl(BYTE cmd, void* buff) {
  if (!MicroSD::available()) {
    return RES_NOTRDY;
  }

  switch (cmd) {
    case CTRL_SYNC:
      return RES_OK;
    
    case GET_SECTOR_COUNT: {
      HAL_SD_CardInfoTypeDef cardInfo;
      if (HAL_SD_GetCardInfo(&MicroSD::hsd, &cardInfo) != HAL_OK) {
        return RES_ERROR;
      }
      *(DWORD*)buff = cardInfo.BlockNbr;
      return RES_OK;
    }
    
    case GET_SECTOR_SIZE: {
      HAL_SD_CardInfoTypeDef cardInfo;
      if (HAL_SD_GetCardInfo(&MicroSD::hsd, &cardInfo) != HAL_OK) {
        return RES_ERROR;
      }
      *(WORD*)buff = cardInfo.BlockSize;
      return RES_OK;
    }
    
    case GET_BLOCK_SIZE:
      *(DWORD*)buff = 1;
      return RES_OK;
      
    default:
      return RES_PARERR;
  }
} 