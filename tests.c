#include "cpu.c"
#include <stdio.h>

static int total_tests = 0;
static int passed_tests = 0;

#define BEGIN_TEST(name)                                                       \
  do {                                                                         \
    printf("%-40s ... ", name);                                                \
    fflush(stdout);                                                            \
    total_tests++;                                                             \
  } while (0)

#define END_TEST(cond)                                                         \
  do {                                                                         \
    if (cond) {                                                                \
      passed_tests++;                                                          \
      printf("OK\n");                                                          \
    } else {                                                                   \
      printf("FAIL\n");                                                        \
    }                                                                          \
  } while (0)

static int flags_equal(uint8_t C, uint8_t Z, uint8_t I, uint8_t D, uint8_t B,
                       uint8_t U, uint8_t V, uint8_t N) {
  return (default_cpu.P.C == C && default_cpu.P.Z == Z &&
          default_cpu.P.I == I && default_cpu.P.D == D &&
          default_cpu.P.B == B && default_cpu.P.U == U &&
          default_cpu.P.V == V && default_cpu.P.N == N);
}

int main(void) {
  printf("Starting 6502 CPU test suite...\n\n");

  BEGIN_TEST("RESET initializes registers and memory");
  reset_cpu();
  int ok_reset =
      (default_cpu.A == 0 && default_cpu.X == 0 && default_cpu.Y == 0 &&
       default_cpu.SP == 0xFF && default_cpu.P.U == 1);
  for (int i = 0; i < 0x10000 && ok_reset; i++)
    if (memory[i] != 0)
      ok_reset = 0;
  END_TEST(ok_reset);

  BEGIN_TEST("LDA sets A and flags correctly");
  LDA(0x42);
  int ok_lda = (default_cpu.A == 0x42 && !default_cpu.P.Z && !default_cpu.P.N);
  LDA(0x00);
  ok_lda &= (default_cpu.P.Z == 1);
  LDA(0xFF);
  ok_lda &= (default_cpu.P.N == 1);
  END_TEST(ok_lda);

  BEGIN_TEST("ADC basic addition and flags");
  reset_cpu();
  LDA(0x10);
  CLC();
  ADC(0x05);
  int ok_adc =
      (default_cpu.A == 0x15 && default_cpu.P.C == 0 && default_cpu.P.V == 0);
  END_TEST(ok_adc);

  BEGIN_TEST("ADC overflow behavior");
  reset_cpu();
  LDA(0x50);
  CLC();
  ADC(0x50);
  int ok_adc_over = (default_cpu.A == 0xA0 && default_cpu.P.V == 1);
  END_TEST(ok_adc_over);

  BEGIN_TEST("Transfers (TAX TAY TXA TYA TXS TSX)");
  reset_cpu();
  LDA(0x7F);
  TAX();
  TAY();
  TXA();
  TYA();
  TXS();
  TSX();
  int ok_transfers =
      (default_cpu.A == default_cpu.X && default_cpu.X == default_cpu.Y &&
       default_cpu.SP == default_cpu.X && default_cpu.P.Z == 0);
  END_TEST(ok_transfers);

  BEGIN_TEST("INX/DEX/INY/DEY modify registers");
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

  BEGIN_TEST("Memory INC/DEC");
  reset_cpu();
  memory[0x200] = 0x42;
  INC(0x200);
  DEC(0x200);
  int ok_mem = (memory[0x200] == 0x42);
  END_TEST(ok_mem);

  BEGIN_TEST("Logic ops AND/ORA/EOR");
  reset_cpu();
  LDA(0xF0);
  AND(0x0F);
  ORA(0xAA);
  EOR(0xFF);
  int ok_logic = (default_cpu.A == 0x55);
  END_TEST(ok_logic);

  BEGIN_TEST("CMP");
  reset_cpu();
  LDA(0x80);
  CMP(0x80);
  int ok_cmp = (default_cpu.P.Z == 1 && default_cpu.P.C == 1);
  END_TEST(ok_cmp);

  BEGIN_TEST("CPX");
  reset_cpu();
  LDX(0x10);
  CPX(0x20);
  int ok_cpx =
      (default_cpu.P.Z == 0 && default_cpu.P.C == 0 && default_cpu.P.N == 1);
  END_TEST(ok_cpx);

  BEGIN_TEST("CPY");
  reset_cpu();
  LDY(0x05);
  CPY(0x04);
  int ok_cpy =
      (default_cpu.P.Z == 0 && default_cpu.P.C == 1 && default_cpu.P.N == 0);
  END_TEST(ok_cpy);

  BEGIN_TEST("Flag manipulation");
  reset_cpu();
  SEC();
  CLD();
  CLI();
  CLV();
  int ok_flags = (default_cpu.P.C == 1 && default_cpu.P.D == 0 &&
                  default_cpu.P.I == 0 && default_cpu.P.V == 0);
  END_TEST(ok_flags);

  BEGIN_TEST("Branching BCC BEQ BPL");
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

  BEGIN_TEST("Stack PHA/PLA");
  reset_cpu();
  LDA(0xAB);
  PHA();
  LDA(0x00);
  LDA(PLA());
  int ok_stack = (default_cpu.A == 0xAB && default_cpu.SP == 0xFF);
  END_TEST(ok_stack);

  BEGIN_TEST("Store instructions");
  reset_cpu();
  LDA(0x12);
  STA(0x0200);
  LDX(0x34);
  STX(0x0201);
  LDY(0x56);
  STY(0x0202);
  int ok_store =
      (memory[0x200] == 0x12 && memory[0x201] == 0x34 && memory[0x202] == 0x56);
  END_TEST(ok_store);

  BEGIN_TEST("BIT");
  reset_cpu();
  LDA(0x40);
  BIT(0xC0);
  int ok_bit =
      (default_cpu.P.Z == 0 && default_cpu.P.V == 1 && default_cpu.P.N == 1);
  END_TEST(ok_bit);

  BEGIN_TEST("SBC");
  reset_cpu();
  LDA(0x10);
  SEC();
  SBC(0x01);
  int ok_sbc = (default_cpu.A == 0x0F && default_cpu.P.C == 1);
  END_TEST(ok_sbc);

  BEGIN_TEST("ASL A");
  reset_cpu();
  LDA(0x40);
  ASL_A();
  int ok_asl =
      (default_cpu.A == 0x80 && default_cpu.P.C == 0 && default_cpu.P.N == 1);
  END_TEST(ok_asl);

  BEGIN_TEST("LSR A");
  reset_cpu();
  LDA(0x01);
  LSR_A();
  int ok_lsr =
      (default_cpu.A == 0x00 && default_cpu.P.C == 1 && default_cpu.P.Z == 1);
  END_TEST(ok_lsr);

  BEGIN_TEST("ROL A");
  reset_cpu();
  LDA(0x80);
  CLC();
  ROL_A();
  int ok_rol =
      (default_cpu.A == 0x00 && default_cpu.P.C == 1 && default_cpu.P.Z == 1);
  END_TEST(ok_rol);

  BEGIN_TEST("ROR A");
  reset_cpu();
  LDA(0x01);
  SEC();
  ROR_A();
  int ok_ror = (default_cpu.A == 0x00 && default_cpu.P.C == 1);
  END_TEST(ok_ror);

  BEGIN_TEST("PHP/PLP");
  reset_cpu();
  SEC();
  SEI();
  PHP();
  CLC();
  CLI();
  PLP();
  int ok_php = (default_cpu.P.C == 1 && default_cpu.P.I == 1);
  END_TEST(ok_php);

  BEGIN_TEST("JSR/RTS");
  reset_cpu();
  default_cpu.PC = 0x3000;
  JSR(0x4000);
  RTS();
  int ok_jsr = (default_cpu.PC == 0x3000);
  END_TEST(ok_jsr);

  BEGIN_TEST("RTI");
  reset_cpu();
  default_cpu.PC = 0x2000;
  SEC();
  PHP();
  push(0x12);
  push(0x34);
  RTI();
  int ok_rti = (default_cpu.PC == 0x1235 && default_cpu.P.C == 1);
  END_TEST(ok_rti);

  BEGIN_TEST("BVC/BVS");
  reset_cpu();
  default_cpu.PC = 0x1000;
  default_cpu.P.V = 0;
  BVC(0x10);
  int ok_bv = (default_cpu.PC == 0x1010);
  default_cpu.P.V = 1;
  BVS(0x10);
  ok_bv &= (default_cpu.PC == 0x1020);
  END_TEST(ok_bv);

  BEGIN_TEST("JMP");
  reset_cpu();
  JMP(0xDEAD);
  int ok_jmp = (default_cpu.PC == 0xDEAD);
  END_TEST(ok_jmp);

  BEGIN_TEST("NOP");
  reset_cpu();
  LDA(0x42);
  NOP();
  int ok_nop = (default_cpu.A == 0x42);
  END_TEST(ok_nop);
  /* --------------------------------------------------------- */
  /* Additional edge-case and correctness tests                */
  /* --------------------------------------------------------- */

  BEGIN_TEST("ADC carry generation");
  reset_cpu();
  LDA(0xFF);
  CLC();
  ADC(0x01);
  int ok_adc_carry =
      (default_cpu.A == 0x00 && default_cpu.P.C == 1 && default_cpu.P.Z == 1);
  END_TEST(ok_adc_carry);

  BEGIN_TEST("ADC negative without overflow");
  reset_cpu();
  LDA(0x80);
  CLC();
  ADC(0x01);
  int ok_adc_neg =
      (default_cpu.A == 0x81 && default_cpu.P.N == 1 && default_cpu.P.V == 0);
  END_TEST(ok_adc_neg);

  BEGIN_TEST("SBC borrow clears carry");
  reset_cpu();
  LDA(0x00);
  SEC();
  SBC(0x01);
  int ok_sbc_borrow =
      (default_cpu.A == 0xFF && default_cpu.P.C == 0 && default_cpu.P.N == 1);
  END_TEST(ok_sbc_borrow);

  BEGIN_TEST("CMP negative result");
  reset_cpu();
  LDA(0x10);
  CMP(0x20);
  int ok_cmp_neg =
      (default_cpu.P.C == 0 && default_cpu.P.N == 1 && default_cpu.P.Z == 0);
  END_TEST(ok_cmp_neg);

  BEGIN_TEST("Zero flag cleared on non-zero load");
  reset_cpu();
  LDA(0x00);
  LDA(0x01);
  int ok_z_clear = (default_cpu.P.Z == 0);
  END_TEST(ok_z_clear);

  BEGIN_TEST("INX wraparound");
  reset_cpu();
  LDX(0xFF);
  INX();
  int ok_inx_wrap = (default_cpu.X == 0x00 && default_cpu.P.Z == 1);
  END_TEST(ok_inx_wrap);

  BEGIN_TEST("DEX wraparound");
  reset_cpu();
  LDX(0x00);
  DEX();
  int ok_dex_wrap = (default_cpu.X == 0xFF && default_cpu.P.N == 1);
  END_TEST(ok_dex_wrap);

  BEGIN_TEST("Stack push/pull order");
  reset_cpu();
  push(0xAA);
  push(0xBB);
  uint8_t v1 = pull();
  uint8_t v2 = pull();
  int ok_stack_order = (v1 == 0xBB && v2 == 0xAA && default_cpu.SP == 0xFF);
  END_TEST(ok_stack_order);

  BEGIN_TEST("PHP sets B flag on stack only");
  reset_cpu();
  PHP();
  uint8_t p = pull();
  int ok_php_b = ((p & 0x10) != 0 && default_cpu.P.B == 0);
  END_TEST(ok_php_b);

  BEGIN_TEST("PLP restores flags correctly");
  reset_cpu();
  push(0xC3); /* N V Z C set */
  PLP();
  int ok_plp = (default_cpu.P.N == 1 && default_cpu.P.V == 1 &&
                default_cpu.P.Z == 1 && default_cpu.P.C == 1);
  END_TEST(ok_plp);

  BEGIN_TEST("ROL uses carry-in");
  reset_cpu();
  LDA(0x7F);
  SEC();
  ROL_A();
  int ok_rol_carry = (default_cpu.A == 0xFE && default_cpu.P.C == 0);
  END_TEST(ok_rol_carry);

  BEGIN_TEST("ROR uses carry-in");
  reset_cpu();
  LDA(0x00);
  SEC();
  ROR_A();
  int ok_ror_carry = (default_cpu.A == 0x00 && default_cpu.P.C == 0);
  END_TEST(ok_ror_carry);

  BEGIN_TEST("Branch backward (negative offset)");
  reset_cpu();
  default_cpu.PC = 0x2000;
  default_cpu.P.Z = 1;
  BEQ(0xF0); /* -16 */
  int ok_branch_back = (default_cpu.PC == 0x1FF0);
  END_TEST(ok_branch_back);

  BEGIN_TEST("JSR pushes correct return address");
  reset_cpu();
  default_cpu.PC = 0x1234;
  JSR(0x4000);
  uint8_t lo = memory[0x01FF];
  uint8_t hi = memory[0x01FE];
  int ok_jsr_stack = (((hi << 8) | lo) == 0x1233);
  END_TEST(ok_jsr_stack);

  BEGIN_TEST("RTI restores PC exactly");
  reset_cpu();
  push(0x00); /* P */
  push(0x78);
  push(0x56);
  RTI();
  int ok_rti_pc = (default_cpu.PC == 0x5678);
  END_TEST(ok_rti_pc);

  BEGIN_TEST("NOP does not modify flags");
  reset_cpu();
  SEC();
  SEI();
  NOP();
  int ok_nop_flags = (default_cpu.P.C == 1 && default_cpu.P.I == 1);
  END_TEST(ok_nop_flags);
  printf("\n6502 TEST SUMMARY: %d / %d tests passed.\n", passed_tests,
         total_tests);

  if (passed_tests == total_tests)
    printf("SUCCESS: All tests passed successfully\n");
  else
    printf("ERROR: Some tests failed.\n");

  return (passed_tests == total_tests) ? 0 : 1;
}
