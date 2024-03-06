#include "inst.h"

#include <stdio.h>

const char *OPCODE_NAMES[] = {
    [NOP] = "nop",       [RETS] = "rets",    [ERROR] = "error",
    [HALT] = "halt",     [RCSR] = "rcsr",    [WCSR] = "wcsr",
    [MOVE] = "move",     [LOADC] = "loadc",  [JUMP] = "jump",
    [IMM] = "imm",       [CALL] = "call",    [IMM2] = "imm2",
    [LOAD] = "load",     [STORE] = "store",  [LOADB] = "loadb",
    [STOREB] = "storeb", [ADD] = "add",      [SUB] = "sub",
    [ADDC] = "addc",     [SUBC] = "subc",    [XOR] = "xor",
    [AND] = "and",       [OR] = "or",        [SHL] = "shl",
    [SHR] = "shr",       [ASR] = "asr",      [IFEQ] = "if.eq",
    [IFNE] = "if.ne",    [IFLT] = "if.lt",   [IFGE] = "if.ge",
    [IFULT] = "if.ult",  [IFUGE] = "if.uge",
};

const char *OpcodeString(Opcode op) { return OPCODE_NAMES[op]; }

const char *FMT_NAMES[] = {
    [FMT_RR] = "RR",   [FMT_LS] = "LS",   [FMT_RI6] = "RI6",
    [FMT_RI8] = "RI8", [FMT_I11] = "I11", [FMT_I12] = "I12",
};

const char *FmtString(Fmt fmt) { return FMT_NAMES[fmt]; }

static Fmt decodeFmt(RawInst inst) {
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

uint16_t signExtend(uint16_t imm, uint8_t bits) {
  uint16_t m = 1 << (bits - 1);
  return (imm ^ m) - m;
}

const char *REG_NAMES[] = {"ra", "a0", "a1", "a2", "s0", "s1", "s2", "s3",
                           "t0", "t1", "t2", "t3", "gp", "bp", "sp"};

const char *regString(int reg) { return REG_NAMES[reg]; }

char *instString(char *buf, int sz, Inst ir) {
  switch (ir.fmt) {
  case FMT_RR:
    snprintf(buf, sz, "%-5s %s, %s", OpcodeString(ir.op), regString(ir.rd),
             regString(ir.rs));
    break;

  case FMT_I11: // fallthrough
  case FMT_I12:
    snprintf(buf, sz, "%-5s %d", OpcodeString(ir.op),
             (int16_t)signExtend(ir.imm, 13));
    break;

  case FMT_RI6:
  case FMT_RI8:
    snprintf(buf, sz, "%-5s %s, %d", OpcodeString(ir.op), regString(ir.rd),
             (int16_t)signExtend(ir.imm, 13));
    break;

  case FMT_LS:
    if (ir.op == LOAD || ir.op == LOADB) {
      snprintf(buf, sz, "%-5s %s, [%s, %d]", OpcodeString(ir.op),
               regString(ir.rd), regString(ir.rs),
               (int16_t)signExtend(ir.imm, 13));
    } else { // STORE
      snprintf(buf, sz, "%-5s [%s, %d], %s", OpcodeString(ir.op),
               regString(ir.rs), (int16_t)signExtend(ir.imm, 13),
               regString(ir.rd));
    }
    break;
  }
  return buf;
}