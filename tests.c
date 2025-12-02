#include <stdio.h>
#include <assert.h>
#include "cpu.c"

int main(void) {
  printf("Starting 6502 CPU basic tests...\n");

  reset_cpu();
  assert(default_cpu.A == 0);
  assert(default_cpu.X == 0);
  assert(default_cpu.Y == 0);
  assert(default_cpu.SP == 0xFF);
  assert(default_cpu.P.U == 1);
  for (int i = 0; i < 0x10000; i++) assert(memory[i] == 0);

  LDA(0x42);
  assert(default_cpu.A == 0x42);
  assert(default_cpu.P.Z == 0);
  assert(default_cpu.P.N == 0);

  CLC();
  ADC(0x10);
  assert(default_cpu.A == (uint8_t)(0x42 + 0x10));
  assert(default_cpu.P.C == 0);
  assert(default_cpu.P.Z == 0);

  TAX();
  assert(default_cpu.X == default_cpu.A);
  assert(default_cpu.P.Z == 0);

  STA(0x200);
  assert(memory[0x200] == default_cpu.A);

  INC(0x200);
  assert(memory[0x200] == (uint8_t)(default_cpu.A + 1));

  LDA(0x00);
  assert(default_cpu.P.Z == 1);
  LDA(0xFF);
  assert(default_cpu.P.N == 1);

  printf("All 6502 CPU basic tests passed successfully!\n");
  return 0;
}
