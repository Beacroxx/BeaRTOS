// This is a template for dynamic executables that can be loaded and executed
// Compile with the build_executable.sh script

#include "dynamic_executable.hpp"

// This is the entry point that will be called when the executable is loaded
int main() {

  printf("starting dynamic task\n");
  
  Scheduler::yieldDelay(1000);

  
    // list files
    DIR dir;
    FILINFO fileInfo;
    FRESULT res = FatFs::openDir("0:", &dir);
    if (res != FR_OK) {
      printf("Failed to open directory: %d\n", res);
      return 0;
    }
    printf("Files in SD card:\n");
    
    while (true) {
      res = FatFs::readDir(&dir, &fileInfo);
      if (res != FR_OK || fileInfo.fname[0] == 0) {
        break;
      }
      
      if (fileInfo.fattrib & AM_DIR) {
        printf("  [DIR] %s\n", fileInfo.fname);
      } else {
        printf("  [FIL] %s (%lu bytes)\n", fileInfo.fname, fileInfo.fsize);
      }
    }

    FatFs::closeDir(&dir);

  while (1) {
    Scheduler::yieldDelay(1000);
    printf("Hello from dynamic executable!");
  }
}