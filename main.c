typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

typedef uint8_t reg8_t;
typedef uint16_t reg16_t;

typedef reg8_t  regA_t; // Accumulator
typedef reg8_t  regX_t; // Index register X
typedef reg8_t  regY_t; // Index register Y
typedef reg8_t  regSP_t;
typedef reg16_t regPC_t; 
typedef reg8_t  regP_t;

#define U8_MAX 0xFF

struct Status {
  uint8_t C:1;
  uint8_t Z:1;
  uint8_t I:1;
  uint8_t D:1;
  uint8_t B:1;
  uint8_t U:1; // always 1?
  uint8_t V:1;
  uint8_t N:1;
};

typedef struct {
  regA_t  A;
  regX_t  X;
  regY_t  Y;
  regSP_t SP;
  regPC_t PC;
  struct Status P;
} cpu6502;

uint8_t memory[0x10000];

void push(cpu6502 *cpu, uint8_t value){
  memory[0x0100 | cpu->SP] = value;
  cpu->SP--;
}

uint8_t pull(cpu6502 *cpu){
  cpu->SP++;
  return memory[0x0100 | cpu->SP];
}

void ADC(cpu6502 *cpu, uint8_t M){
  // C Z V N affected
  uint16_t sum = cpu->A + M + (cpu->P.C ? 1 : 0);

  cpu->P.C = (sum > U8_MAX);
  cpu->P.V = (~(cpu->A ^ M) & (cpu->A ^ sum) & 0x80) != 0;

  cpu->A = sum;

  cpu->P.Z = (cpu->A == 0);
  cpu->P.N = (cpu->A & 0x80) != 0;
}

void AND(cpu6502 *cpu, uint8_t M){
  // Z N affected
  cpu->A = cpu->A & M;
  cpu->P.Z = (cpu->A == 0);
  cpu->P.N = (cpu->A & 0x80) != 0;
}

void BRK(cpu6502 *cpu){
  cpu->P.B = 1;
}

void CLC(cpu6502 *cpu){
  cpu->P.C = 0;
}

void CLD(cpu6502 *cpu){
  cpu->P.D = 0;
}

void CLI(cpu6502 *cpu){
  cpu->P.I = 0;
}

void CLV(cpu6502 *cpu){
  cpu->P.V = 0;
}

void CMP(cpu6502 *cpu, uint8_t M){
  // C Z N affected
  uint8_t result = cpu->A - M;
  cpu->P.C = (cpu->A >= M);
  cpu->P.Z = (cpu->A == M);
  cpu->P.N = (cpu->A & 0x80) != 0; 
}

void CPX(cpu6502 *cpu, uint8_t M){
  // C Z N affected
  uint8_t result = cpu->X - M;
  cpu->P.C = (cpu->X >= M);
  cpu->P.Z = (cpu->X == M);
  cpu->P.N = (result & 0x80) != 0; 
}

void CPY(cpu6502 *cpu, uint8_t M){
  // C Z N affected
  uint8_t result = cpu->Y - M;
  cpu->P.C = (cpu->Y >= M);
  cpu->P.Z = (cpu->Y == M);
  cpu->P.N = (result & 0x80) != 0; 
}

void DEC(cpu6502 *cpu, uint16_t addr){
  // Z N affected
  uint8_t value = memory[addr];
  value = (value - 1) & U8_MAX;

  memory[addr] = value;
  cpu->P.Z = (value == 0);
  cpu->P.N = (value & 0x80) != 0;
}

void DEX(cpu6502 *cpu){
  // Z N affected
  uint8_t value = cpu->X;
  value = (value - 1) & U8_MAX;

  cpu->X  = value;
  cpu->P.Z = (value == 0);
  cpu->P.N = (value & 0x80) != 0;
}

void DEY(cpu6502 *cpu){
  // Z N affected
  uint8_t value = cpu->Y;
  value = (value - 1) & U8_MAX;

  cpu->Y  = value;
  cpu->P.Z = (value == 0);
  cpu->P.N = (value & 0x80) != 0;
}

void EOR(cpu6502 *cpu, uint8_t M){
  // Z N affected
  cpu->A = cpu->A ^ M;
  cpu->P.Z = (cpu->A == 0);
  cpu->P.N = (cpu->A & 0x80) != 0;
}

void INC(cpu6502 *cpu, uint16_t addr){
  // Z N affected
  uint8_t value = memory[addr];
  value = (value + 1) & U8_MAX;

  memory[addr] = value;
  cpu->P.Z = (value == 0);
  cpu->P.N = (value & 0x80) != 0;
}

void INX(cpu6502 *cpu){
  // Z N affected
  uint8_t value = cpu->X;
  value = (value + 1) & U8_MAX;

  cpu->X  = value;
  cpu->P.Z = (value == 0);
  cpu->P.N = (value & 0x80) != 0;
}

void INY(cpu6502 *cpu){
  // Z N affected
  uint8_t value = cpu->Y;
  value = (value + 1) & U8_MAX;

  cpu->Y  = value;
  cpu->P.Z = (value == 0);
  cpu->P.N = (value & 0x80) != 0;
}

void JMP(cpu6502 *cpu, uint16_t addr){
  cpu->PC = addr;
}

void LDA(cpu6502 *cpu, uint8_t M){
  // Z N affected
  cpu->A = M;
  cpu->P.Z = (cpu->A == 0);
  cpu->P.N = (cpu->A & 0x80) != 0;
}

void LDX(cpu6502 *cpu, uint8_t M){
  // Z N affected
  cpu->X = M;
  cpu->P.Z = (cpu->X == 0);
  cpu->P.N = (cpu->X & 0x80) != 0;
}


void LDY(cpu6502 *cpu, uint8_t M){
  // Z N affected
  cpu->A = M;
  cpu->P.Z = (cpu->Y == 0);
  cpu->P.N = (cpu->Y & 0x80) != 0;
}

void ORA(cpu6502 *cpu, uint8_t M){
  // Z N affected
  cpu->A = cpu->A | M;
  cpu->P.Z = (cpu->A == 0);
  cpu->P.N = (cpu->A & 0x80) != 0;
}

void PHA(cpu6502 *cpu){
  push(cpu, cpu->A);
}

void PHP(cpu6502 *cpu){
  // push(cpu, cpu->P); not sure how to do for now
}

uint8_t PLA(cpu6502 *cpu){
  return pull(cpu);
}

// TODO: IMPLEMENT PLP

void SEC(cpu6502 *cpu){
  cpu->P.C = 1;
}

void SED(cpu6502 *cpu){
  cpu->P.D = 1;
}

void SEI(cpu6502 *cpu){
  cpu->P.I = 1;
}

void STA(cpu6502 *cpu, uint8_t addr){
  memory[addr] = cpu->A;
}

void STX(cpu6502 *cpu, uint8_t addr){
  memory[addr] = cpu->X;
}

void STY(cpu6502 *cpu, uint8_t addr){
  memory[addr] = cpu->Y;
}

void TAX(cpu6502 *cpu){
  // Z N affected
  cpu->X = cpu->A;
  cpu->P.Z = (cpu->X == 0);
  cpu->P.N = (cpu->X & 0x80) != 0;
}

void TAY(cpu6502 *cpu){
  // Z N affected
  cpu->Y = cpu->A;
  cpu->P.Z = (cpu->Y == 0);
  cpu->P.N = (cpu->Y & 0x80) != 0;
}

void TSX(cpu6502 *cpu){
  // Z N affected
  cpu->X = cpu->SP;
  cpu->P.Z = (cpu->X == 0);
  cpu->P.N = (cpu->X & 0x80) != 0;
}

void TXA(cpu6502 *cpu){
  // Z N affected
  cpu->A = cpu->X;
  cpu->P.Z = (cpu->A == 0);
  cpu->P.N = (cpu->A & 0x80) != 0;
}

void TXS(cpu6502 *cpu){
  // Z N affected
  cpu->SP = cpu->X;
  cpu->P.Z = (cpu->SP == 0);
  cpu->P.N = (cpu->SP & 0x80) != 0;
}

void TYA(cpu6502 *cpu){
  // Z N affected
  cpu->A = cpu->Y;
  cpu->P.Z = (cpu->A == 0);
  cpu->P.N = (cpu->A & 0x80) != 0;
}

