#include "bus.h"
#include "cpu.h"
#include "inst.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// runRj32Emu runs the emulator for the specified number of cycles with a
// default set of IO devices and the specified program and data memory.
// Returns the error code.
int runRj32Emu(uint64_t maxCycles, const uint16_t *progMem, int progLength,
               const uint16_t *dataMem, int dataLength, bool trace) {
  CPU cpu;
  Device devices[] = {
      {
          .address = 0xFF00,
          .size = 1,
          .handler = stdoutWriter,
          .context = NULL,
      },
      {
          .address = 0,
          .size = 0xffff,
          .handler = memoryHandler,
          .context = malloc(0x10000 * sizeof(uint16_t)),
      },
  };

  cpuInit(&cpu, trace);
  cpuWriteProgMem(&cpu, progMem, progLength);
  cpuInitBusDevices(&cpu, devices, sizeof(devices) / sizeof(devices[0]));
  cpuWriteDataMem(&cpu, dataMem, dataLength);
  cpuRun(&cpu, maxCycles);

  // make sure to free the data memory allocated
  free(devices[1].context);

  if (cpu.error) {
    return cpu.reg[1] || 1;
  }
  if (cpu.halt) {
    return 0;
  }
  fprintf(stderr, "Program failed to terminate\n");
  return 1;
}
