#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum Opcode {
  NOP,
  RETS,
  ERROR,
  HALT,
  RCSR,
  WCSR,
  MOVE,
  LOADC,
  JUMP,
  IMM,
  CALL,
  IMM2,
  LOAD,
  STORE,
  LOADB,
  STOREB,
  ADD,
  SUB,
  ADDC,
  SUBC,
  XOR,
  AND,
  OR,
  SHL,
  SHR,
  ASR,
  IFEQ,
  IFNE,
  IFLT,
  IFGE,
  IFULT,
  IFUGE,
} Opcode;

const char *OPCODE_NAMES[] = {
    [NOP] = "NOP",   [RETS] = "RETS",   [ERROR] = "ERROR", [HALT] = "HALT",
    [RCSR] = "RCSR", [WCSR] = "WCSR",   [MOVE] = "MOVE",   [LOADC] = "LOADC",
    [JUMP] = "JUMP", [IMM] = "IMM",     [CALL] = "CALL",   [IMM2] = "IMM2",
    [LOAD] = "LOAD", [STORE] = "STORE", [LOADB] = "LOADB", [STOREB] = "STOREB",
    [ADD] = "ADD",   [SUB] = "SUB",     [ADDC] = "ADDC",   [SUBC] = "SUBC",
    [XOR] = "XOR",   [AND] = "AND",     [OR] = "OR",       [SHL] = "SHL",
    [SHR] = "SHR",   [ASR] = "ASR",     [IFEQ] = "IFEQ",   [IFNE] = "IFNE",
    [IFLT] = "IFLT", [IFGE] = "IFGE",   [IFULT] = "IFULT", [IFUGE] = "IFUGE",
};

typedef enum Fmt {
  FMT_RR = 0b00,
  FMT_LS = 0b10,
  FMT_RI6 = 0b11,
  FMT_RI8 = 0b001,
  FMT_I11 = 0b0101,
  FMT_I12 = 0b1101,
} Fmt;

const char *FMT_NAMES[] = {
    [FMT_RR] = "RR",   [FMT_LS] = "LS",   [FMT_RI6] = "RI6",
    [FMT_RI8] = "RI8", [FMT_I11] = "I11", [FMT_I12] = "I12",
};

typedef struct RawInst {
  union {
    struct {
      uint16_t fmt : 2;
      uint16_t op : 5;
      uint16_t : 1;
      uint16_t rs : 4;
      uint16_t rd : 4;
    } rr;
    struct {
      uint16_t : 2;
      uint16_t op : 4;
      uint16_t imm : 6;
      uint16_t rd : 4;
    } ri6;
    struct {
      uint16_t fmt : 3;
      uint16_t op : 1;
      uint16_t imm : 8;
      uint16_t rd : 4;
    } ri8;
    struct {
      uint16_t : 2;
      uint16_t op : 2;
      uint16_t imm : 4;
      uint16_t rs : 4;
      uint16_t rd : 4;
    } ls;
    struct {
      uint16_t : 3;
      uint16_t op : 2;
      uint16_t imm : 11;
    } i11;
    struct {
      uint16_t fmt : 4;
      uint16_t imm : 12;
    } i12;
    uint16_t raw;
  };
} RawInst;

typedef struct Inst {
  uint16_t fmt : 4;
  uint16_t op : 5;
  uint16_t rd : 4;
  uint16_t rs : 4;
  uint16_t imm : 13;
} Inst;

Fmt decodeFmt(RawInst inst) {
  if (inst.rr.fmt == FMT_RI8) {
    if (inst.ri8.fmt == FMT_RI8) {
      return FMT_RI8;
    }
    if (inst.i12.fmt == FMT_I11) {
      return FMT_I11;
    }
    return FMT_I12;
  }
  return inst.rr.fmt;
}

uint16_t signExtend(uint16_t imm, uint8_t bits) {
  uint16_t m = 1 << (bits - 1);
  return (imm ^ m) - m;
  // return (imm << (16 - bits)) >> (16 - bits);
}

Inst decode(RawInst inst) {
  Fmt fmt = decodeFmt(inst);
  switch (fmt) {
  case FMT_RR:
    return (Inst){
        .fmt = fmt,
        .op = inst.rr.op,
        .rd = inst.rr.rd,
        .rs = inst.rr.rs,
        .imm = 0,
    };
  case FMT_LS:
    return (Inst){
        .fmt = fmt,
        .op = inst.ls.op | 0b01100,
        .rd = inst.ls.rd,
        .rs = inst.ls.rs,
        .imm = inst.ls.imm,
    };
  case FMT_RI6:
    return (Inst){
        .fmt = fmt,
        .op = inst.ri6.op | 0b10000,
        .rd = inst.ri6.rd,
        .rs = 0,
        .imm = signExtend(inst.ri6.imm, 6),
    };
  case FMT_RI8:
    return (Inst){
        .fmt = fmt,
        .op = inst.ri8.op | 0b00110,
        .rd = inst.ri8.rd,
        .rs = 0,
        .imm = signExtend(inst.ri8.imm, 8),
    };
  case FMT_I11:
    return (Inst){
        .fmt = fmt,
        .op = inst.i11.op | 0b01000,
        .rd = 0,
        .rs = 0,
        .imm = signExtend(inst.i11.imm, 11),
    };
  case FMT_I12:
    return (Inst){
        .fmt = fmt,
        .op = IMM,
        .rd = 0,
        .rs = 0,
        .imm = signExtend(inst.i12.imm, 12),
    };
  }
  // unreachable
  return (Inst){.fmt = fmt, .op = 0, .rd = 0, .rs = 0, .imm = 0};
}

typedef struct Bus {
  uint16_t data;
  uint16_t address;
  bool WE;
} Bus;

typedef bool (*BusHandler)(void *context, Bus *bus, uint16_t subAddress);

typedef struct Device {
  uint16_t address;
  uint16_t size;
  BusHandler handler;
  void *context;
} Device;

typedef struct IOBus {
  Bus bus;
  Device *busDevices;
  int numDevices;
} IOBus;

void ioBusInit(IOBus *ioBus, Device *devices, int numDevices) {
  memset(ioBus, 0, sizeof(IOBus));
  ioBus->busDevices = devices;
  ioBus->numDevices = numDevices;
}

bool ioBusTransaction(IOBus *ioBus, uint16_t address, uint16_t data, bool WE) {
  ioBus->bus.address = address;
  ioBus->bus.data = data;
  ioBus->bus.WE = WE;
  for (int i = 0; i < ioBus->numDevices; i++) {
    uint16_t deviceAddress = ioBus->busDevices[i].address;
    uint16_t deviceSize = ioBus->busDevices[i].size;
    if (address >= deviceAddress && address < deviceAddress + deviceSize) {
      if (ioBus->busDevices[i].handler(ioBus->busDevices[i].context,
                                       &ioBus->bus, address - deviceAddress)) {
        return true;
      }
    }
  }
  return false;
}

typedef struct CPU {
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

void cpuRun(CPU *cpu, int cycles) {
  uint64_t endCycle = cpu->cycles + cycles;

  for (; cpu->cycles < endCycle; cpu->cycles++) {
    Inst ir = cpu->prog[cpu->pc];
    if (cpu->trace) {
      // todo: better trace format
      fprintf(stderr, "%04x: %3s %5s %2d %2d %d (%d, %d)\n", cpu->pc,
              FMT_NAMES[ir.fmt], OPCODE_NAMES[ir.op], ir.rd, ir.rs,
              (int16_t)cpuImm(cpu, ir.imm), (int16_t)cpu->reg[ir.rd],
              (int16_t)cpuRsval(cpu, ir));
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
      fprintf(stderr, "  imm rsval: %d\n", (int16_t)cpuRsval(cpu, ir));
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
      // todo: implement
      fprintf(stderr, "Unknown opcode: %d %s\n", ir.op, OPCODE_NAMES[ir.op]);
      assert(false);
      break;
    }

    if (cpu->trace) {
      // todo: implement
      if (ir.op == STORE) {
        fprintf(stderr, "  mem[%04x] <- %d\n",
                cpu->reg[ir.rs] + cpuOffset(cpu, ir.imm),
                (int16_t)cpu->reg[ir.rd]);
      } else if (ir.op == LOAD) {
        fprintf(stderr, "  rd <- %d <- mem[%04x]\n", (int16_t)cpu->reg[ir.rd],
                cpu->reg[ir.rs] + cpuOffset(cpu, ir.imm));
      } else if (ir.op == IMM || ir.op == IMM2) {
        fprintf(stderr, "  imm <- %d\n", (int16_t)cpu->imm);
      } else {
        fprintf(stderr, "  rd <- %d\n", (int16_t)cpu->reg[ir.rd]);
      }
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

bool stdoutWriter(void *context, Bus *bus, uint16_t address) {
#pragma unused(context)
#pragma unused(address)
  if (bus->WE) {
    char c = bus->data;
    if (c == '\r') {
      printf("\033D");
    } else {
      printf("%c", c);
    }
    return true;
  }
  return false;
}

bool memoryHandler(void *context, Bus *bus, uint16_t address) {
  uint16_t *mem = context;
  if (bus->WE) {
    mem[address] = bus->data;
    return true;
  }
  bus->data = mem[address];
  return true;
}

int run(uint64_t maxCycles, const uint16_t *progMem, int progLength,
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

  if (cpu.error) {
    return cpu.reg[1] || 1;
  }
  if (cpu.halt) {
    return 0;
  }
  fprintf(stderr, "Program failed to terminate\n");
  return 1;
}

typedef struct testcase {
  const char *name;
  int len;
  const uint16_t *prog;
} testcase;

int testEmu() {
  assert(sizeof(RawInst) == 2);

  const testcase tests[] = {
      {"add", 7,
       (uint16_t[]){0x1051, 0x2031, 0x1240, 0x1083, 0x12af, 0x0008, 0x000c}},
      {"sub", 9,
       (uint16_t[]){0x1051, 0x2031, 0x1244, 0x10af, 0x0008, 0x1087, 0x102f,
                    0x0008, 0x000c}},
      {"and", 9,
       (uint16_t[]){0x1061, 0x20a1, 0x1254, 0x10af, 0x0008, 0x2357, 0x222f,
                    0x0008, 0x000c}},
      {"or", 9,
       (uint16_t[]){0x1061, 0x20a1, 0x1258, 0x13af, 0x0008, 0x205b, 0x22ef,
                    0x0008, 0x000c}},
      {"xor", 12,
       (uint16_t[]){0x1061, 0x2021, 0x1250, 0x112f, 0x0008, 0x1250, 0x11af,
                    0x0008, 0x10d3, 0x116f, 0x0008, 0x000c}},
      {"shl", 25,
       (uint16_t[]){0x1031, 0x2001, 0x3011, 0x4021, 0x101f, 0x10ef, 0x0008,
                    0x125c, 0x10ef, 0x0008, 0x105f, 0x11af, 0x0008, 0x135c,
                    0x132f, 0x0008, 0x1031, 0x109f, 0x132f, 0x0008, 0x1031,
                    0x145c, 0x132f, 0x0008, 0x000c}},
      {"shr", 29, (uint16_t[]){0x1061, 0x2001, 0x3011, 0x4021, 0x1023, 0x11af,
                               0x0008, 0x1260, 0x11af, 0x0008, 0x1063, 0x10ef,
                               0x0008, 0x1360, 0x106f, 0x0008, 0x10c1, 0x10a3,
                               0x10ef, 0x0008, 0x10c1, 0x1460, 0x10ef, 0x0008,
                               0x1d61, 0x10a3, 0x1dab, 0x0008, 0x000c}},
      {"asr", 36, (uint16_t[]){0x1061, 0x2001, 0x3011, 0x4021, 0x1027, 0x11af,
                               0x0008, 0x1264, 0x11af, 0x0008, 0x1067, 0x10ef,
                               0x0008, 0x1364, 0x106f, 0x0008, 0x10c1, 0x10a7,
                               0x10ef, 0x0008, 0x10c1, 0x1464, 0x10ef, 0x0008,
                               0x1f81, 0x10a7, 0x1faf, 0x0008, 0x10a7, 0x1fef,
                               0x0008, 0x1f61, 0x1067, 0x1eef, 0x0008, 0x000c}},
      {"imm", 52,
       (uint16_t[]){0xfffd, 0x3001, 0x3fd3, 0x33ef, 0x0008, 0x1751, 0x121f,
                    0x2491, 0x1258, 0x754d, 0x126f, 0x0008, 0x5461, 0x008d,
                    0x5003, 0x00cd, 0x51af, 0x0008, 0x53f1, 0x003d, 0x5fef,
                    0x0008, 0x50a1, 0x52af, 0x006d, 0x5903, 0x52af, 0x0008,
                    0x7e1d, 0x5121, 0x7e1d, 0x54af, 0x0008, 0x0015, 0x4018,
                    0x57f1, 0x0015, 0x2018, 0x2444, 0x20ef, 0x0008, 0x2201,
                    0x1001, 0x123d, 0x4341, 0x4206, 0x002d, 0x3102, 0x123d,
                    0x3d2f, 0x0008, 0x000c}},
      {"loadstore", 63,
       (uint16_t[]){0x0021, 0x1051, 0x2002, 0x216b, 0x0008, 0x1006, 0x2002,
                    0x216f, 0x0008, 0x1071, 0x10f6, 0x20f2, 0x21ef, 0x0008,
                    0x2001, 0x03c3, 0x2002, 0x21ef, 0x0008, 0x1011, 0x2021,
                    0x3031, 0x4041, 0x5051, 0x6061, 0x0001, 0x3036, 0x1016,
                    0x6066, 0x2026, 0x4046, 0x5056, 0x1ff1, 0x2ff1, 0x3ff1,
                    0x4ff1, 0x5ff1, 0x6ff1, 0x5052, 0x1012, 0x6062, 0x2022,
                    0x4042, 0x3032, 0x106f, 0x0008, 0x20af, 0x0008, 0x30ef,
                    0x0008, 0x412f, 0x0008, 0x516f, 0x0008, 0x61af, 0x0008,
                    0x3032, 0x0025, 0x0008, 0x3036, 0x0025, 0x0008, 0x000c}},
      {"jump", 15,
       (uint16_t[]){0x0105, 0x0008, 0x0140, 0x0240, 0x00e5, 0x0008, 0x2021,
                    0xff45, 0x0008, 0x1031, 0xff65, 0x0008, 0x016f, 0x0008,
                    0x000c}},
      {"call", 9,
       (uint16_t[]){0x0095, 0x116f, 0x0008, 0x000c, 0x0008, 0x1051, 0x0020,
                    0x0008, 0x000c}},
      {"if.eq", 14,
       (uint16_t[]){0x1021, 0x2031, 0x3031, 0x1268, 0x0008, 0x10eb, 0x0008,
                    0x2368, 0x0025, 0x0008, 0x20eb, 0x0025, 0x0008, 0x000c}},
      {"if.ne", 14,
       (uint16_t[]){0x1021, 0x2031, 0x3031, 0x226c, 0x0008, 0x10af, 0x0008,
                    0x216c, 0x0025, 0x0008, 0x20af, 0x0025, 0x0008, 0x000c}},
      {"if.lt", 17,
       (uint16_t[]){0x1ff1, 0x2001, 0x2170, 0x0008, 0x2270, 0x0008, 0x2ff3,
                    0x0008, 0x2033, 0x0008, 0x1270, 0x0025, 0x0008, 0x1073,
                    0x0025, 0x0008, 0x000c}},
      {"if.ge", 19,
       (uint16_t[]){0x1ff1, 0x2001, 0x1274, 0x0008, 0x1037, 0x0008, 0x2174,
                    0x0025, 0x0008, 0x2274, 0x0025, 0x0008, 0x2037, 0x0025,
                    0x0008, 0x1ff7, 0x0025, 0x0008, 0x000c}},
      {"if.ult", 17,
       (uint16_t[]){0x1001, 0x2ff1, 0x2178, 0x0008, 0x2278, 0x0008, 0x203b,
                    0x0008, 0x2ffb, 0x0008, 0x1278, 0x0025, 0x0008, 0x107b,
                    0x0025, 0x0008, 0x000c}},
      {"if.uge", 19,
       (uint16_t[]){0x1001, 0x2ff1, 0x127c, 0x0008, 0x1fff, 0x0008, 0x217c,
                    0x0025, 0x0008, 0x227c, 0x0025, 0x0008, 0x2fff, 0x0025,
                    0x0008, 0x103f, 0x0025, 0x0008, 0x000c}},
  };
  int failed = 0;
  for (int i = 0; i < (int)(sizeof(tests) / sizeof(tests[0])); i++) {
    const testcase *tc = &tests[i];
    fprintf(stderr, "\n%2d: %s\n", i, tc->name);
    int retval = run(1000000, tc->prog, tc->len, NULL, 0, true);
    if (retval) {
      fprintf(stderr, "FAIL: %d\n", retval);
      failed++;
    } else {
      fprintf(stderr, "PASS\n");
    }
  }
  if (failed) {
    fprintf(stderr, "\nFAIL: %d tests\n", failed);
    return 1;
  }

  fprintf(stderr, "\nALL PASS\n");

  return 0;
}

int main() { return testEmu(); }