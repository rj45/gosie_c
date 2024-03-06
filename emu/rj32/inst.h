#ifndef INST_H
#define INST_H

#include <stdint.h>

// Opcode is the list of all possible instructions. Many instructions come in
// immediate or register-register form, so they are only listed here once.
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

// OpcodeString returns the name of each instruction.
const char *OpcodeString(Opcode op);

// Fmt is the list of all possible instruction formats.
typedef enum Fmt {
  FMT_RR = 0b00,
  FMT_LS = 0b10,
  FMT_RI6 = 0b11,
  FMT_RI8 = 0b001,
  FMT_I11 = 0b0101,
  FMT_I12 = 0b1101,
} Fmt;

// FmtString returns the name of the given instruction format.
const char *FmtString(Fmt fmt);

// RawInst is a translation from/to the raw instruction format using bitfields.
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
      uint16_t fmt : 2;
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
      uint16_t fmt : 2;
      uint16_t op : 2;
      uint16_t imm : 4;
      uint16_t rs : 4;
      uint16_t rd : 4;
    } ls;
    struct {
      uint16_t fmt : 3;
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

// Inst is a decoded instruction.
typedef struct Inst {
  uint16_t fmt : 4;
  uint16_t op : 5;
  uint16_t rd : 4;
  uint16_t rs : 4;
  uint16_t imm : 13;
} Inst;

// signExtend returns the sign-extended value by copying the value of the
// `bits`th bit into the rest of the upper bits of `imm`.
uint16_t signExtend(uint16_t imm, uint8_t bits);

// decode returns the decoded instruction.
Inst decode(RawInst inst);

// regString returns the string name of the given register.
const char *regString(int reg);

// instString returns the string representation of the given instruction.
char *instString(char *buf, int sz, Inst ir);

#endif