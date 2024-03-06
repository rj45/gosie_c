#ifndef CPU_H
#define CPU_H

#include "bus.h"
#include "inst.h"

#include <stdbool.h>
#include <stdint.h>

// CPU represents the working state of an rj32 CPU.
typedef struct CPU {
  // count of cycles since the start of the program
  uint64_t cycles;

  // Data bus
  IOBus bus;

  // register file
  uint16_t reg[16];

  // program counter, points into `prog`
  uint16_t pc;

  // skip flag for tracking if next instruction should be skipped
  bool skip;

  // immediate register and state
  uint16_t imm;
  bool immValid;
  bool immExpire;

  // halt and error output signals
  bool halt;
  bool error;

  // emit detailed instruction traces
  bool trace;

  // pre-decoded program memory
  Inst prog[65536];
} CPU;

// cpuInit initializes the CPU.
void cpuInit(CPU *cpu, bool trace);

// cpuInitBusDevices initializes the IO bus with the given device list.
void cpuInitBusDevices(CPU *cpu, Device *devices, int numDevices);

// cpuWriteProgMem writes the program into the CPU's program memory.
// The program is pre-decoded for speed such that the regular 16 bit
// instructions are unpacked into 32 bits. This is fine because the program
// memory is only 128kb, and unpacking will make it only 256kb. If you're
// porting this emulator to a 32-bit architecture you probably don't want to do
// this, and instead decode the program during execution.
void cpuWriteProgMem(CPU *cpu, const uint16_t *progMem, int progLength);

// cpuWriteDataMem writes the data into the CPU's IO bus, which can write it
// into data memory, or into other devices on the IO bus.
void cpuWriteDataMem(CPU *cpu, const uint16_t *dataMem, int dataLength);

// cpuRun runs the CPU for the specified number of cycles. It's faster to run
// at least a few cycles at a time, but cycles can be 1 if you want to single
// step. If the CPU halts, it will return early and cpu->halt or cpu->error will
// be set. If it returns without those signals being set, it indicates the CPU
// hit the cycle limit.
void cpuRun(CPU *cpu, int cycles);

#endif