#ifndef DISKIO_MICROSD_H
#define DISKIO_MICROSD_H

#include "ff.h"
#include "diskio.h"

#ifdef __cplusplus
extern "C" {
#endif

DSTATUS MMC_disk_status(void);
DSTATUS MMC_disk_initialize(void);
DRESULT MMC_disk_read(BYTE* buff, LBA_t sector, UINT count);
DRESULT MMC_disk_write(const BYTE* buff, LBA_t sector, UINT count);
DRESULT MMC_disk_ioctl(BYTE cmd, void* buff);

#ifdef __cplusplus
}
#endif

#endif // DISKIO_MICROSD_H 