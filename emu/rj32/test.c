#include "emurj.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct testcase {
  const char *name;
  int len;
  const uint16_t *prog;
} testcase;

int main() {
  // these tests came from using customasm on the rj32 tests
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
    int retval = runRj32Emu(1000000, tc->prog, tc->len, NULL, 0, true);
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