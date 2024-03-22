#include "gosie.h"

#include <assert.h>

const OpDef opDefs[] = {
    [OP_INVALID] = {"invalid", INPUT_NONE}, [OP_INT] = {"int", INPUT_INT},
    [OP_ADD] = {"add", INPUT_TWO},          [OP_SUB] = {"sub", INPUT_TWO},
    [OP_ERROR] = {"error", INPUT_ONE},
};

const OpDef *opDef(Op op) { return &opDefs[op]; }

void irInit(IR *ir, AST *ast) { *ir = (IR){.ast = ast}; }
void irFree(IR *ir) { arrfree(ir->instrs); }

InstrID irAddInstr(IR *ir, Op op, NodeID astNode) {
  InstrID id = arrlen(ir->instrs);
  arrput(ir->instrs, ((Instr){.op = op, .astNode = astNode}));
  return id;
}

InstrID irSetInt(IR *ir, InstrID instr, uint64_t value) {
  assert(ir->instrs[instr].op == OP_INT);
  ir->instrs[instr].intConst = value;
  return instr;
}

InstrID irSetInput1(IR *ir, InstrID instr, InstrID input) {
  assert(opDefs[ir->instrs[instr].op].inputType == INPUT_ONE);
  ir->instrs[instr].inputs[0] = input;
  return instr;
}

InstrID irSetInput2(IR *ir, InstrID instr, InstrID input1, InstrID input2) {
  assert(opDefs[ir->instrs[instr].op].inputType == INPUT_TWO);
  ir->instrs[instr].inputs[0] = input1;
  ir->instrs[instr].inputs[1] = input2;
  return instr;
}

char *irPrintInstr(char *start, char *end, IR *ir, InstrID instr) {
  const Instr *i = &ir->instrs[instr];
  const char *opName = opDefs[i->op].name;
  switch (opDefs[i->op].inputType) {
  case INPUT_NONE:
    return seprintf(start, end, "v%d = %s", instr, opName);
  case INPUT_ONE:
    return seprintf(start, end, "v%d = %s v%d", instr, opName, i->inputs[0]);
  case INPUT_TWO:
    return seprintf(start, end, "v%d = %s v%d, v%d", instr, opName,
                    i->inputs[0], i->inputs[1]);
  case INPUT_INT:
    return seprintf(start, end, "v%d = %s %llu", instr, opName, i->intConst);

  default:
    assert(0); // unreachable
  }
}

char *irDump(IR *ir, char *start, char *end) {
  char *cur = start;
  for (size_t i = 0; i < (size_t)arrlen(ir->instrs); i++) {
    cur = irPrintInstr(cur, end, ir, i);
    cur = seprintf(cur, end, "\n");
  }
  return cur;
}

void irBuilderInit(IRBuilder *builder, IR *ir) {
  *builder = (IRBuilder){.ir = ir, .ast = ir->ast};
}

void irBuilderFree(IRBuilder *builder) {}

// FIXME: can read past the end of this array in the case of a missing token
Op tokenTypeOp[] = {
    [TK_ADD] = OP_ADD,
    [TK_SUB] = OP_SUB,
};

static InstrID addInstr(IRBuilder *builder, NodeID astNode) {
  switch (builder->ast->nodes[astNode].type) {
  case LITERAL: {
    Token token = builder->ast->nodes[astNode].token;
    InstrID instr = irAddInstr(builder->ir, OP_INT, astNode);
    uint64_t value =
        strtoull(srcTokenStringNoNull(builder->ast->src, token), NULL, 10);
    irSetInt(builder->ir, instr, value);
    return instr;
  }
  case BINARY: {
    ChildIter iter = astNewChildIter(builder->ast, astNode);
    NodeID left = astCurChild(iter);
    NodeID right = astCurChild(astNextChild(iter));
    InstrID leftInstr = addInstr(builder, left);
    InstrID rightInstr = addInstr(builder, right);
    Op op = tokenTypeOp[builder->ast->nodes[astNode].token.type];

    InstrID instr = irAddInstr(builder->ir, op, astNode);
    irSetInput2(builder->ir, instr, leftInstr, rightInstr);
    return instr;
  }
  default:
    assert(0); // unreachable
  }
}

void irBuilderBuild(IRBuilder *builder) {
  InstrID result = addInstr(builder, astRootNode(builder->ast));
  irSetInput1(builder->ir, irAddInstr(builder->ir, OP_ERROR, NO_NODE), result);
}