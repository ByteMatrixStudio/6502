#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "cpu.c"

#undef STA
#define STA(addr) STA_os(addr)

#define IO_PUTCHAR 0xFF00
#define PROGRAM_START 0x8000

void STA_os(uint16_t addr) {
  if (addr == IO_PUTCHAR) {
    putchar(default_cpu.A);
    fflush(stdout);
  } else {
    memory[addr] = default_cpu.A;
  }
}

int load_bin(const char *path, uint16_t load_addr) {
  FILE *f = fopen(path, "rb");
  if (!f) {
    perror("fopen");
    return -1;
  }

  fseek(f, 0, SEEK_END);
  long size = ftell(f);
  rewind(f);

  if (size <= 0 || load_addr + size > 0x10000) {
    fprintf(stderr, "invalid binary size\n");
    fclose(f);
    return -1;
  }

  fread(&memory[load_addr], 1, (size_t)size, f);
  fclose(f);

  memory[0xFFFC] = load_addr & 0xFF;
  memory[0xFFFD] = (load_addr >> 8) & 0xFF;

  return 0;
}

void cpu_step(cpu6502 *cpu) {
  uint8_t opcode = memory[cpu->PC++];

  switch (opcode) {
  case 0xA9: { // LDA immediate
    uint8_t value = memory[cpu->PC++];
    LDA_c(cpu, value);
    break;
  }

  case 0x8D: { // STA absolute
    uint8_t lo = memory[cpu->PC++];
    uint8_t hi = memory[cpu->PC++];
    uint16_t addr = ((uint16_t)hi << 8) | lo;
    STA(addr);
    break;
  }
  case 0xA2: { // LDX immediate
    uint8_t value = memory[cpu->PC++];
    LDX_c(cpu, value);
    break;
  }

  case 0xE8: // INX
    INX_c(cpu);
    break;

  case 0xE0: { // CPX immediate
    uint8_t value = memory[cpu->PC++];
    CPX_c(cpu, value);
    break;
  }

  case 0x90: { // BCC relative
    int8_t offset = (int8_t)memory[cpu->PC++];
    BCC_c(cpu, offset);
    break;
  }

  case 0x4C: { // JMP absolute
    uint8_t lo = memory[cpu->PC++];
    uint8_t hi = memory[cpu->PC++];
    uint16_t addr = ((uint16_t)hi << 8) | lo;
    JMP_c(cpu, addr);
    break;
  }
  case 0x8A: { // TXA
    TXA_c(cpu);
    break;
  }
  case 0x69: { // ADC immediate
    uint8_t value = memory[cpu->PC++];
    ADC_c(cpu, value);
    break;
  }
  case 0xCA: { // DEX
    DEX_c(cpu);
    break;
  }
  case 0xD0: { // BNE relative
    int8_t offset = (int8_t)memory[cpu->PC++];
    BNE_c(cpu, offset);
    break;
  }
  case 0x20: { // JSR absolute
    uint8_t lo = memory[cpu->PC++];
    uint8_t hi = memory[cpu->PC++];
    uint16_t addr = ((uint16_t)hi << 8) | lo;
    JSR_c(cpu, addr);
    break;
  }
  case 0x60: { // RTS
    RTS_c(cpu);
    break;
  }
  case 0x00: // BRK
    return;

  default:
    printf("Unknown opcode: %02X at %04X\n", opcode, cpu->PC - 1);
    return;
  }
}

void run_cpu(cpu6502 *cpu) {
  for (;;) {
    uint8_t opcode = memory[cpu->PC];
    cpu_step(cpu);

    if (opcode == 0x00) { /* BRK */
      break;
    }
  }
}

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: %s program.bin\n", argv[0]);
    return 1;
  }

  reset_cpu();

  if (load_bin(argv[1], PROGRAM_START) != 0) {
    return 1;
  }

  default_cpu.PC = (uint16_t)memory[0xFFFC] | ((uint16_t)memory[0xFFFD] << 8);

  run_cpu(&default_cpu);

  return 0;
}
