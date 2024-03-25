#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>


enum AsmResult {
  Ok = 0,
  Failed = 1,
  NullAssembly = 2,
  NullBinary = 3,
  NullBinaryLen = 4,
};
typedef uint32_t AsmResult;

AsmResult assemble_str_to_binary(const char *assembly,
                                 const unsigned char **binary,
                                 size_t *binary_len);

AsmResult free_binary(const unsigned char *binary, size_t binary_len);
