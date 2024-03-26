#include "emu/rj32/emurj.h"
#include "gosie.h"
#include "libcustomasm/libcustomasm.h"

int compileAndRun(const char *source) {
  Source src = (Source){source, strlen(source)};
  Tokenizer tokenizer;
  AST ast;
  IR ir;
  IRBuilder builder;

  tokenizerInit(&tokenizer, src);

  astInit(&ast, src);
  parse(&tokenizer, &ast);

  irInit(&ir, &ast);
  irBuilderInit(&builder, &ir);
  irBuilderBuild(&builder);

  char assembly[65536];

  FILE *cpudef = fopen("cpudefs/rj32_cpudef.asm", "r");
  if (cpudef == NULL) {
    fprintf(stderr, "error: could not open cpudefs/rj32_cpudef.asm\n");
    return 1;
  }

  size_t size = fread(assembly, 1, sizeof(assembly) - 1, cpudef);
  fclose(cpudef);
  if (size > 65536 - 1024) {
    fprintf(stderr, "error: cpudefs/rj32_cpudef.asm too large\n");
    return 1;
  }
  assembly[size] = '\n';
  assembly[size + 1] = '\0';
  char *start = assembly + size + 1;
  char *end = assembly + sizeof(assembly) - 1;

  genCode(start, end, &ir);

  const unsigned char *binary = NULL;

  AsmResult result = assemble_str_to_binary(assembly, &binary, &size);
  if (result != Ok) {
    fprintf(stderr, "error: assembly failed\n");
    return 1;
  }

  int code = runRj32Emu(100000, (uint16_t *)binary, size / 2, NULL, 0, true);

  free_binary(binary, size);

  irFree(&ir);
  astFree(&ast);

  return code;
}