#include "emurj.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, const char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <file>\n", argv[0]);
    exit(1);
  }

  FILE *f = fopen(argv[1], "rb");
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET); /* same as rewind(f); */

  uint16_t *rom = malloc(fsize + 1);
  fread(rom, fsize, 1, f);
  fclose(f);

  int romsize = fsize / 2;
  uint16_t *data = NULL;
  int datasize = 0;

  if (romsize > 0x10000) {
    data = rom + 0x10000;
    datasize = romsize - 0x10000;
    romsize = 0x10000;
  }

  int ret = runRj32Emu(1000000, rom, romsize, data, datasize, true);
  if (ret) {
    exit(ret);
  }

  return 0;
}