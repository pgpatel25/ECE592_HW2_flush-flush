#define _GNU_SOURCE
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <stdint.h>
#include "../cacheutils.h"

size_t start = 0;
size_t keystate = 0;
size_t kpause = 0;
// void flushandreload(void* addr)
// {
//   size_t time = rdtsc();
//   flush(addr);
//   size_t delta = rdtsc() - time;
//   if (delta >= 444)
//   {
//     if (kpause > 1000)
//     {
//       printf("Cache Hit (%zu) after %10lu cycles, t=%10lu us\n", delta, kpause, (time-start)/2600);
//       keystate = (keystate+1) % 2;
//     }
//     kpause = 0;
//   }
//   else
//     kpause++;
// }

void flushandreload(void* addr)
{
  static size_t samples = 0;
  static size_t above = 0;
  static size_t minimum = (size_t)-1;
  static size_t maximum = 0;

  size_t time = rdtsc();
  flush(addr);
  size_t delta = rdtsc() - time;

  if (delta < minimum) minimum = delta;
  if (delta > maximum) maximum = delta;
  if (delta >= 564) above++;

  samples++;

  if (samples == 10000)
  {
    printf("samples=%zu  >=564=%zu  min=%zu  max=%zu\n",
           samples, above, minimum, maximum);
    fflush(stdout);

    samples = 0;
    above = 0;
    minimum = (size_t)-1;
    maximum = 0;
  }
}

int main(int argc, char** argv)
{
  if (argc != 3)
  {
    fprintf(stderr, "Usage: %s FILE HEX_OFFSET\n", argv[0]);
    return 1;
  }
  char* name = argv[1];
  char* offsetp = argv[2];
  unsigned int offset = 0;
  if (sscanf(offsetp, "%x", &offset) != 1) {
  fprintf(stderr, "Invalid hexadecimal offset: %s\n", offsetp);
  return 1;
}
  int fd = open(name,O_RDONLY);
  if (fd == -1)
  {
    perror(name);
    return 2;
  }
  unsigned char* addr = (unsigned char*)mmap(0, 64*1024*1024, PROT_READ, MAP_SHARED, fd, 0);
  if (addr == (void*)-1)
    return 3;
  start = rdtsc();
  while(1)
  {
    flushandreload(addr + offset);
    for (int i = 0; i < 30; ++i)
      sched_yield();
  }
  return 0;
}
