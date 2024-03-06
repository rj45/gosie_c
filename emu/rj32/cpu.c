#include "cpu.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void cpuInit(CPU *cpu, bool trace) {
  memset(cpu, 0, sizeof(CPU));
  cpu->trace = trace;
}

void cpuInitBusDevices(CPU *cpu, Device *devices, int numDevices) {
  ioBusInit(&cpu->bus, devices, numDevices);
}

void cpuWriteProgMem(CPU *cpu, const uint16_t *progMem, int progLength) {
  for (int i = 0; i < 65536; i++) {
    if (i < progLength) {
      cpu->prog[i] = decode((RawInst){.raw = progMem[i]});
    } else {
      cpu->prog[i] = decode((RawInst){.raw = 0});
    }
  }
}

void cpuWriteDataMem(CPU *cpu, const uint16_t *dataMem, int dataLength) {
  for (int i = 0; i < dataLength; i++) {
    ioBusTransaction(&cpu->bus, i, dataMem[i], true);
  }
}

static uint16_t cpuImm(const CPU *cpu, uint16_t imm) {
  if (cpu->immValid) {
    return cpu->imm | (imm & 0b1111);
  }
  return signExtend(imm, 13);
}

static uint16_t cpuRsval(CPU *cpu, Inst ir) {
  if (ir.fmt == FMT_RR) {
    return cpu->reg[ir.rs];
  }
  return cpuImm(cpu, ir.imm);
}

static uint16_t cpuOffset(const CPU *cpu, uint16_t imm) {
  if (cpu->immValid) {
    return cpu->imm | (imm & 0b1111);
  }
  return (imm & 0b1111);
}

static char *preTrace(char *buf, int sz, CPU *cpu, Inst ir) {
  snprintf(buf, sz, "%-15s", instString(buf, sz, ir));
  char *ptr = buf + strlen(buf);
  sz -= strlen(buf);

  switch (ir.fmt) {
  case FMT_RR:
    snprintf(ptr, sz, "(%s:%d %s:%d)", regString(ir.rd),
             (int16_t)cpu->reg[ir.rd], regString(ir.rs),
             (int16_t)cpu->reg[ir.rs]);
    break;
  case FMT_I11:
    snprintf(ptr, sz, "(pc:%04x rsval:%d)", cpu->pc,
             (int16_t)cpuRsval(cpu, ir));
    break;
  case FMT_I12:
    snprintf(ptr, sz, "(rsval:%d)", (int16_t)cpuRsval(cpu, ir));
    break;
  case FMT_RI6:
  case FMT_RI8:
    snprintf(ptr, sz, "(%s:%d rsval:%d)", regString(ir.rd),
             (int16_t)cpu->reg[ir.rd], (int16_t)cpuRsval(cpu, ir));
    break;
  case FMT_LS:
    snprintf(ptr, sz, "(%s:%d off:%d)", regString(ir.rs),
             (int16_t)cpu->reg[ir.rs], (int16_t)cpuOffset(cpu, ir.imm));
    break;
  }

  return buf;
}

static char *postTrace(char *buf, int sz, CPU *cpu, Inst ir) {
  switch (ir.fmt) {
  case FMT_RR:
  case FMT_RI6:
  case FMT_RI8:
    if (ir.op >= IFEQ) {
      snprintf(buf, sz, "skip <- %s", cpu->skip ? "true" : "false");
    } else {
      snprintf(buf, sz, "%s <- %d", regString(ir.rd), (int16_t)cpu->reg[ir.rd]);
    }
    break;

  case FMT_I11:
    snprintf(buf, sz, "pc <- %04x", cpu->pc + 1);
    break;

  case FMT_I12:
    snprintf(buf, sz, "imm <- %d", (int16_t)cpu->imm);
    break;

  case FMT_LS: {
    int address = (cpu->reg[ir.rs] + cpuOffset(cpu, ir.imm)) & 0xffff;
    if (ir.op == LOAD) {
      snprintf(buf, sz, "%s <- %d <- mem[%04x]\n", regString(ir.rd),
               (int16_t)cpu->reg[ir.rd], address);
    } else if (ir.op == STORE) {
      snprintf(buf, sz, "mem[%04x] <- %d\n", address, (int16_t)cpu->reg[ir.rd]);
    }
  } break;
  }

  return buf;
}

void cpuRun(CPU *cpu, int cycles) {
  char buf[256];
  uint64_t endCycle = cpu->cycles + cycles;

  for (; cpu->cycles < endCycle; cpu->cycles++) {
    Inst ir = cpu->prog[cpu->pc];
    if (cpu->trace) {
      fprintf(stderr, "%04x: %s\n", cpu->pc, preTrace(buf, 256, cpu, ir));
    }

    switch (ir.op) {
    case NOP:
      // do nothing
      break;

    case HALT:
      cpu->halt = true;
      return;

    case ERROR:
      cpu->error = true;
      return;

    case RCSR:
      // temporary jump instruction
      cpu->pc = cpu->reg[ir.rd];
      if (cpu->trace) {
        fprintf(stderr, "  temp jump, PC <- %04x\n", cpu->pc + 1);
      }
      break;

    case MOVE:
      cpu->reg[ir.rd] = cpuRsval(cpu, ir);
      break;

    case IMM: // fall through
    case IMM2:
      cpu->imm = cpuRsval(cpu, ir) << 4;
      cpu->immValid = true;
      cpu->immExpire = true;
      break;

    case CALL:
      cpu->reg[0] = cpu->pc;
      if (ir.fmt == FMT_RR) {
        cpu->pc = cpu->reg[ir.rs];
      } else {
        cpu->pc += cpuImm(cpu, ir.imm);
      }
      break;

    case JUMP:
      if (ir.fmt == FMT_RR) {
        cpu->pc = cpu->reg[ir.rs];
      } else {
        cpu->pc += cpuImm(cpu, ir.imm);
      }
      break;

    case LOAD:
      ioBusTransaction(&cpu->bus, cpu->reg[ir.rs] + cpuOffset(cpu, ir.imm), 0,
                       false);
      cpu->reg[ir.rd] = cpu->bus.bus.data;
      break;

    case STORE:
      ioBusTransaction(&cpu->bus, cpu->reg[ir.rs] + cpuOffset(cpu, ir.imm),
                       cpu->reg[ir.rd], true);
      break;

    case ADD:
      cpu->reg[ir.rd] = cpu->reg[ir.rd] + cpuRsval(cpu, ir);
      break;

    case SUB:
      cpu->reg[ir.rd] = cpu->reg[ir.rd] - cpuRsval(cpu, ir);
      break;

    case XOR:
      cpu->reg[ir.rd] = cpu->reg[ir.rd] ^ cpuRsval(cpu, ir);
      break;

    case AND:
      cpu->reg[ir.rd] = cpu->reg[ir.rd] & cpuRsval(cpu, ir);
      break;

    case OR:
      cpu->reg[ir.rd] = cpu->reg[ir.rd] | cpuRsval(cpu, ir);
      break;

    case SHL:
      cpu->reg[ir.rd] = cpu->reg[ir.rd] << (cpuRsval(cpu, ir) & 0xf);
      break;

    case SHR:
      cpu->reg[ir.rd] = cpu->reg[ir.rd] >> (cpuRsval(cpu, ir) & 0xf);
      break;

    case ASR: {
      int16_t a = (int16_t)cpu->reg[ir.rd];
      int16_t b = (int16_t)(cpuRsval(cpu, ir) & 0xf);
      cpu->reg[ir.rd] = (uint16_t)(a >> b);
    }; break;

    case IFEQ:
      if (cpu->reg[ir.rd] != cpuRsval(cpu, ir)) {
        cpu->skip = true;
      }
      break;

    case IFNE:
      if (cpu->reg[ir.rd] == cpuRsval(cpu, ir)) {
        cpu->skip = true;
      }
      break;

    case IFLT: {
      int16_t a = (int16_t)cpu->reg[ir.rd];
      int16_t b = (int16_t)cpuRsval(cpu, ir);
      if (a >= b) {
        cpu->skip = true;
      }
    } break;

    case IFGE: {
      int16_t a = (int16_t)cpu->reg[ir.rd];
      int16_t b = (int16_t)cpuRsval(cpu, ir);
      if (a < b) {
        cpu->skip = true;
      }
    } break;

    case IFULT:
      if (cpu->reg[ir.rd] >= cpuRsval(cpu, ir)) {
        cpu->skip = true;
      }
      break;

    case IFUGE:
      if (cpu->reg[ir.rd] < cpuRsval(cpu, ir)) {
        cpu->skip = true;
      }
      break;

    default:
      fprintf(stderr, "Unknown opcode: %d %s\n", ir.op, OpcodeString(ir.op));
      assert(false);
      break;
    }

    if (cpu->trace) {
      fprintf(stderr, "  %s\n", postTrace(buf, sizeof(buf), cpu, ir));
    }

    if (!cpu->immExpire) {
      cpu->imm = 0;
      cpu->immValid = false;
    }
    cpu->immExpire = false;
    cpu->pc++;

    if (cpu->skip) {
      if (cpu->prog[cpu->pc].op == IMM) {
        cpu->pc++;
      }
      cpu->pc++;
      cpu->skip = false;
    }
  }
}