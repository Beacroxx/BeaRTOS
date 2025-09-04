// This is a template for dynamic executables that can be loaded and executed
// Compile with the build_executable.sh script

#include "dynamic_executable.hpp"

// This is the entry point that will be called when the executable is loaded
int main() {

  printf("starting dynamic task\n");

  while (1) {
    printf("Hello from dynamic executable!");
    sleep(1000);
  }
}