#pragma once

#if ENABLE_MICROSD && ENABLE_FATFS

#include "ff.h"
#include "ffconf.h"

class FatFs {
  public:
    static FRESULT mount(const char* path);
    static FRESULT unmount(const char* path);
    static FRESULT openFile(const char* path, FIL* file, BYTE mode);
    static FRESULT closeFile(FIL* file);
    static FRESULT readFile(FIL* file, void* buffer, UINT bytesToRead, UINT* bytesRead);
    static FRESULT writeFile(FIL* file, const void* buffer, UINT bytesToWrite, UINT* bytesWritten);
    static FRESULT seekFile(FIL* file, FSIZE_t offset);
    static FRESULT truncateFile(FIL* file);
    static FRESULT syncFile(FIL* file);
    static FRESULT openDir(const char* path, DIR* dir);
    static FRESULT closeDir(DIR* dir);
    static FRESULT readDir(DIR* dir, FILINFO* fileInfo);
    static FRESULT findFirst(DIR* dir, FILINFO* fileInfo, const char* path, const char* pattern);
    static FRESULT findNext(DIR* dir, FILINFO* fileInfo);
    static FRESULT makeDir(const char* path);
    static FRESULT remove(const char* path);
    static FRESULT rename(const char* oldPath, const char* newPath);
    static FRESULT getFileInfo(const char* path, FILINFO* fileInfo);
    static FRESULT changeAttributes(const char* path, BYTE attr, BYTE mask);
    static FRESULT setFileTime(const char* path, const FILINFO* fileInfo);
    static FRESULT changeDir(const char* path);
    static FRESULT changeDrive(const char* path);
    static FRESULT getCurrentDir(char* buffer, UINT len);
    static FRESULT getFreeSpace(const char* path, DWORD* freeClusters, FATFS** fs);
    static FRESULT getVolumeLabel(const char* path, char* label, DWORD* vsn);
    static FRESULT setVolumeLabel(const char* label);
    static FRESULT forwardFile(FIL* file, UINT(*func)(const BYTE*,UINT), UINT bytesToForward, UINT* bytesForwarded);
    static FRESULT expandFile(FIL* file, FSIZE_t size, BYTE option);
    static FRESULT format(const char* path, const MKFS_PARM* options, void* work, UINT len);
    static FRESULT partition(BYTE pdrv, const LBA_t ptbl[], void* work);
    static FRESULT setCodePage(WORD cp);
    static int putChar(char c, FIL* file);
    static int putString(const char* str, FIL* file);
    static int printf(FIL* file, const char* str, ...);
    static char* getString(char* buffer, int len, FIL* file);

  private:
    static FATFS fs;
    static bool isMounted;
}; 

#endif
