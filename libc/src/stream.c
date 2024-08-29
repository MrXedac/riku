#include "stdio.h"

FILE f_stdin = {
  .fd = 0,
  .modeflags = 0x1,
  .position = 0,
};

FILE f_stdout = {
  .fd = 1,
  .modeflags = 0x2,
  .position = 0,
};

FILE f_stderr = {
  .fd = 2,
  .modeflags = 0x2,
  .position = 0,
};
