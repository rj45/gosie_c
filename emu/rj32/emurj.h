#ifndef EMURJ_H
#define EMURJ_H

#include <stdint.h>
#include <stdbool.h>

int runRj32Emu(uint64_t maxCycles, const uint16_t *progMem, int progLength,
               const uint16_t *dataMem, int dataLength, bool trace);


#endif