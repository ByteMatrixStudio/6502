#include <stdio.h>
#include "cpu.c"

static int total_tests = 0;
static int passed_tests = 0;

#define BEGIN_TEST(name)                        \
  do {                                           \
    printf("%-40s ... ", name);                  \
    fflush(stdout);                              \
    total_tests++;                               \
  } while (0)

#define END_TEST(cond)                           \
  do {                                           \
    if (cond) {                                 \
      passed_tests++;                           \
      printf("OK\n");                           \
    } else {                                    \
      printf("FAIL\n");                         \
    }                                            \
  } while (0)

static int flags_equal(uint8_t C, uint8_t Z, uint8_t I, uint8_t D,
                       uint8_t B, uint8_t U, uint8_t V, uint8_t N) {
  return (default_cpu.P.C == C &&
          default_cpu.P.Z == Z &&
          default_cpu.P.I == I &&
          default_cpu.P.D == D &&
          default_cpu.P.B == B &&
          default_cpu.P.U == U &&
          default_cpu.P.V == V &&
          default_cpu.P.N == N);
}

int main(void) {
  printf("Starting 6502 CPU test suite...\n\n");

  // ----------------------------------------------------------
  BEGIN_TEST("RESET initializes registers and memory");
  reset_cpu();
  int ok_reset = (default_cpu.A == 0 &&
                  default_cpu.X == 0 &&
                  default_cpu.Y == 0 &&
                  default_cpu.SP == 0xFF &&
                  default_cpu.P.U == 1);
  for (int i = 0; i < 0x10000 && ok_reset; i++)
    if (memory[i] != 0) ok_reset = 0;
  END_TEST(ok_reset);

  // ----------------------------------------------------------
  BEGIN_TEST("LDA sets A and flags correctly");
  LDA(0x42);
  int ok_lda = (default_cpu.A == 0x42 && !default_cpu.P.Z && !default_cpu.P.N);
  LDA(0x00);
  ok_lda &= (default_cpu.P.Z == 1);
  LDA(0xFF);
  ok_lda &= (default_cpu.P.N == 1);
  END_TEST(ok_lda);

  // ----------------------------------------------------------
  BEGIN_TEST("ADC basic addition and flags");
  reset_cpu();
  LDA(0x10);
  CLC();
  ADC(0x05);
  int ok_adc = (default_cpu.A == 0x15 && default_cpu.P.C == 0 &&
                default_cpu.P.V == 0);
  END_TEST(ok_adc);

  // ----------------------------------------------------------
  BEGIN_TEST("ADC overflow behavior");
  reset_cpu();
  LDA(0x50);
  CLC();
  ADC(0x50);
  int ok_adc_over = (default_cpu.A == 0xA0 && default_cpu.P.V == 1);
  END_TEST(ok_adc_over);

  // ----------------------------------------------------------
  BEGIN_TEST("Transfers (TAX, TAY, TXA, TYA, TXS, TSX)");
  reset_cpu();
  LDA(0x7F);
  TAX();
  TAY();
  TXA();
  TYA();
  TXS();
  TSX();
  int ok_transfers =
      (default_cpu.A == default_cpu.X &&
       default_cpu.X == default_cpu.Y &&
       default_cpu.SP == default_cpu.X &&
       default_cpu.P.Z == 0);
  END_TEST(ok_transfers);

  // ----------------------------------------------------------
  BEGIN_TEST("INX/DEX/INY/DEY modify registers correctly");
  reset_cpu();
  LDX(0x00);
  INX();
  DEX();
  LDY(0xFF);
  INY();
  DEY();
  int ok_incs =
      (default_cpu.X == 0 && default_cpu.Y == 0xFF && default_cpu.P.Z == 0);
  END_TEST(ok_incs);

  // ----------------------------------------------------------
  BEGIN_TEST("Memory INC/DEC operations");
  reset_cpu();
  memory[0x200] = 0x42;
  INC(0x200);
  DEC(0x200);
  int ok_mem = (memory[0x200] == 0x42);
  END_TEST(ok_mem);

  // ----------------------------------------------------------
  BEGIN_TEST("Logic ops (AND/ORA/EOR)");
  reset_cpu();
  LDA(0xF0);
  AND(0x0F);
  ORA(0xAA);
  EOR(0xFF);
  int ok_logic = (default_cpu.A == 0x55);
  END_TEST(ok_logic);

  // ----------------------------------------------------------
  // ✅ Corrected comparison tests
  BEGIN_TEST("Comparison ops (CMP)");
  reset_cpu();
  LDA(0x80);
  CMP(0x80);
  int ok_cmp = (default_cpu.P.Z == 1 && default_cpu.P.C == 1);
  END_TEST(ok_cmp);

  BEGIN_TEST("Comparison ops (CPX)");
  reset_cpu();
  LDX(0x10);
  CPX(0x20);
  int ok_cpx = (default_cpu.P.Z == 0 && default_cpu.P.C == 0 && default_cpu.P.N == 1);
  END_TEST(ok_cpx);

  BEGIN_TEST("Comparison ops (CPY)");
  reset_cpu();
  LDY(0x05);
  CPY(0x04);
  int ok_cpy = (default_cpu.P.Z == 0 && default_cpu.P.C == 1 && default_cpu.P.N == 0);
  END_TEST(ok_cpy);

  // ----------------------------------------------------------
  BEGIN_TEST("Flag manipulation CLC/SEC CLD/SED CLI/SEI CLV");
  reset_cpu();
  SEC();
  CLD();
  CLI();
  CLV();
  int ok_flags =
      (default_cpu.P.C == 1 && default_cpu.P.D == 0 && default_cpu.P.I == 0 &&
       default_cpu.P.V == 0);
  END_TEST(ok_flags);

  // ----------------------------------------------------------
  BEGIN_TEST("Branching (BCC BEQ BPL)");
  reset_cpu();
  default_cpu.PC = 0x1000;
  default_cpu.P.C = 0;
  BCC(0x10);
  int ok_branch = (default_cpu.PC == 0x1010);
  default_cpu.P.Z = 1;
  BEQ(0x20);
  ok_branch &= (default_cpu.PC == 0x1030);
  default_cpu.P.N = 0;
  BPL(0x10);
  ok_branch &= (default_cpu.PC == 0x1040);
  END_TEST(ok_branch);

  // ----------------------------------------------------------
  BEGIN_TEST("Push/Pull stack");
  reset_cpu();
  LDA(0xAB);
  PHA();
  LDA(0x00);
  LDA(PLA());
  int ok_stack = (default_cpu.A == 0xAB && default_cpu.SP == 0xFF);
  END_TEST(ok_stack);

  // ----------------------------------------------------------
  BEGIN_TEST("Store/Load memory (STA/STX/STY)");
  reset_cpu();
  LDA(0x12);
  STA(0x0200);
  LDX(0x34);
  STX(0x0201);
  LDY(0x56);
  STY(0x0202);
  int ok_store = (memory[0x200] == 0x12 && memory[0x201] == 0x34 &&
                  memory[0x202] == 0x56);
  END_TEST(ok_store);

  // ----------------------------------------------------------
  printf("\n6502 TEST SUMMARY: %d / %d tests passed.\n",
         passed_tests, total_tests);

  if (passed_tests == total_tests)
    printf("✅ All tests passed successfully!\n");
  else
    printf("❌ Some tests failed.\n");

  return (passed_tests == total_tests) ? 0 : 1;
}
