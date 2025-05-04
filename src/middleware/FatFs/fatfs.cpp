#if ENABLE_MICROSD && ENABLE_FATFS

#include "fatfs.hpp"
#include <cstdarg>
#include <stdio.h>

FATFS FatFs::fs;
bool FatFs::isMounted = false;

FRESULT FatFs::mount(const char* path) {
  FRESULT res = f_mount(&fs, path, 1);
  isMounted = (res == FR_OK);
  return res;
}

FRESULT FatFs::unmount(const char* path) {
  FRESULT res = f_mount(nullptr, path, 0);
  isMounted = (res != FR_OK);
  return res;
}

FRESULT FatFs::openFile(const char* path, FIL* file, BYTE mode) {
  return f_open(file, path, mode);
}

FRESULT FatFs::closeFile(FIL* file) {
  return f_close(file);
}

FRESULT FatFs::readFile(FIL* file, void* buffer, UINT bytesToRead, UINT* bytesRead) {
  return f_read(file, buffer, bytesToRead, bytesRead);
}

FRESULT FatFs::writeFile(FIL* file, const void* buffer, UINT bytesToWrite, UINT* bytesWritten) {
  return f_write(file, buffer, bytesToWrite, bytesWritten);
}

FRESULT FatFs::seekFile(FIL* file, FSIZE_t offset) {
  return f_lseek(file, offset);
}

FRESULT FatFs::syncFile(FIL* file) {
  return f_sync(file);
}

FRESULT FatFs::openDir(const char* path, DIR* dir) {
  return f_opendir(dir, path);
}

FRESULT FatFs::closeDir(DIR* dir) {
  return f_closedir(dir);
}

FRESULT FatFs::readDir(DIR* dir, FILINFO* fileInfo) {
  return f_readdir(dir, fileInfo);
}

FRESULT FatFs::makeDir(const char* path) {
  return f_mkdir(path);
}

FRESULT FatFs::remove(const char* path) {
  return f_unlink(path);
}

FRESULT FatFs::rename(const char* oldPath, const char* newPath) {
  return f_rename(oldPath, newPath);
}

FRESULT FatFs::getFileInfo(const char* path, FILINFO* fileInfo) {
  return f_stat(path, fileInfo);
}

FRESULT FatFs::getFreeSpace(const char* path, DWORD* freeClusters, FATFS** fs) {
  return f_getfree(path, freeClusters, fs);
}

FRESULT FatFs::truncateFile(FIL* file) {
  return f_truncate(file);
}

FRESULT FatFs::findFirst(DIR* dir, FILINFO* fileInfo, const char* path, const char* pattern) {
  return f_findfirst(dir, fileInfo, path, pattern);
}

FRESULT FatFs::findNext(DIR* dir, FILINFO* fileInfo) {
  return f_findnext(dir, fileInfo);
}

FRESULT FatFs::changeAttributes(const char* path, BYTE attr, BYTE mask) {
  return f_chmod(path, attr, mask);
}

FRESULT FatFs::setFileTime(const char* path, const FILINFO* fileInfo) {
  return f_utime(path, fileInfo);
}

FRESULT FatFs::changeDir(const char* path) {
  return f_chdir(path);
}

FRESULT FatFs::changeDrive(const char* path) {
  return f_chdrive(path);
}

FRESULT FatFs::getCurrentDir(char* buffer, UINT len) {
  return f_getcwd(buffer, len);
}

FRESULT FatFs::getVolumeLabel(const char* path, char* label, DWORD* vsn) {
  return f_getlabel(path, label, vsn);
}

FRESULT FatFs::setVolumeLabel(const char* label) {
  return f_setlabel(label);
}

FRESULT FatFs::forwardFile(FIL* file, UINT(*func)(const BYTE*,UINT), UINT bytesToForward, UINT* bytesForwarded) {
  return f_forward(file, func, bytesToForward, bytesForwarded);
}

FRESULT FatFs::expandFile(FIL* file, FSIZE_t size, BYTE option) {
  return f_expand(file, size, option);
}

FRESULT FatFs::format(const char* path, const MKFS_PARM* options, void* work, UINT len) {
  return f_mkfs(path, options, work, len);
}

FRESULT FatFs::partition(BYTE pdrv, const LBA_t ptbl[], void* work) {
  return f_fdisk(pdrv, ptbl, work);
}

FRESULT FatFs::setCodePage(WORD cp) {
  return f_setcp(cp);
}

int FatFs::putChar(char c, FIL* file) {
  return f_putc(c, file);
}

int FatFs::putString(const char* str, FIL* file) {
  return f_puts(str, file);
}

int FatFs::printf(FIL* file, const char* str, ...) {
  va_list args;
  va_start(args, str);
  int result = f_printf(file, str, args);
  va_end(args);
  return result;
}

char* FatFs::getString(char* buffer, int len, FIL* file) {
  return f_gets(buffer, len, file);
}

#endif