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

static uint8_t memory[0x10000];
static cpu6502 default_cpu = {0};

#define push(value) push_c(&default_cpu, value)
void push_c(cpu6502 *cpu, uint8_t value){
  memory[0x0100 | cpu->SP] = value;
  cpu->SP--;
}

#define pull() pull_c(&default_cpu)
uint8_t pull_c(cpu6502 *cpu){
  cpu->SP++;
  return memory[0x0100 | cpu->SP];
}

#define ADC(M) ADC_c(&default_cpu, M)
void ADC_c(cpu6502 *cpu, uint8_t M){
  // C Z V N affected
  uint16_t sum = cpu->A + M + (cpu->P.C ? 1 : 0);

  cpu->P.C = (sum > U8_MAX);
  cpu->P.V = (~(cpu->A ^ M) & (cpu->A ^ sum) & 0x80) != 0;

  cpu->A = sum;

  cpu->P.Z = (cpu->A == 0);
  cpu->P.N = (cpu->A & 0x80) != 0;
}

#define AND(M) AND_c(&default_cpu, M)
void AND_c(cpu6502 *cpu, uint8_t M){
  // Z N affected
  cpu->A = cpu->A & M;
  cpu->P.Z = (cpu->A == 0);
  cpu->P.N = (cpu->A & 0x80) != 0;
}

#define BRK() BRK_c(&default_cpu)
void BRK_c(cpu6502 *cpu){
  cpu->P.B = 1;
}

#define CLC() CLC_c(&default_cpu)
void CLC_c(cpu6502 *cpu){
  cpu->P.C = 0;
}

#define CLD() CLD_c(&default_cpu)
void CLD_c(cpu6502 *cpu){
  cpu->P.D = 0;
}

#define CLI() CLI_c(&default_cpu)
void CLI_c(cpu6502 *cpu){
  cpu->P.I = 0;
}

#define CLV() CLV_c(&default_cpu)
void CLV_c(cpu6502 *cpu){
  cpu->P.V = 0;
}

#define CMP(M) CMP_c(&default_cpu, M)
void CMP_c(cpu6502 *cpu, uint8_t M){
  // C Z N affected
  uint8_t result = cpu->A - M;
  cpu->P.C = (cpu->A >= M);
  cpu->P.Z = (cpu->A == M);
  cpu->P.N = (cpu->A & 0x80) != 0; 
}

#define CPX(M) CPX_c(&default_cpu, M) 
void CPX_c(cpu6502 *cpu, uint8_t M){
  // C Z N affected
  uint8_t result = cpu->X - M;
  cpu->P.C = (cpu->X >= M);
  cpu->P.Z = (cpu->X == M);
  cpu->P.N = (result & 0x80) != 0; 
}

#define CPY(M) CPY_c(&default_cpu, M)
void CPY_c(cpu6502 *cpu, uint8_t M){
  // C Z N affected
  uint8_t result = cpu->Y - M;
  cpu->P.C = (cpu->Y >= M);
  cpu->P.Z = (cpu->Y == M);
  cpu->P.N = (result & 0x80) != 0; 
}

#define DEC(addr) DEC_c(&default_cpu, addr)
void DEC_c(cpu6502 *cpu, uint16_t addr){
  // Z N affected
  uint8_t value = memory[addr];
  value = (value - 1) & U8_MAX;

  memory[addr] = value;
  cpu->P.Z = (value == 0);
  cpu->P.N = (value & 0x80) != 0;
}

#define DEX() DEX_c(&default_cpu)
void DEX_c(cpu6502 *cpu){
  // Z N affected
  uint8_t value = cpu->X;
  value = (value - 1) & U8_MAX;

  cpu->X  = value;
  cpu->P.Z = (value == 0);
  cpu->P.N = (value & 0x80) != 0;
}

#define DEY() DEY_c(&default_cpu)
void DEY_c(cpu6502 *cpu){
  // Z N affected
  uint8_t value = cpu->Y;
  value = (value - 1) & U8_MAX;

  cpu->Y  = value;
  cpu->P.Z = (value == 0);
  cpu->P.N = (value & 0x80) != 0;
}

#define EOR(M) EOR(&default_cpu, M)
void EOR_c(cpu6502 *cpu, uint8_t M){
  // Z N affected
  cpu->A = cpu->A ^ M;
  cpu->P.Z = (cpu->A == 0);
  cpu->P.N = (cpu->A & 0x80) != 0;
}

#define INC(addr) INC_c(&default_cpu, addr)
void INC_c(cpu6502 *cpu, uint16_t addr){
  // Z N affected
  uint8_t value = memory[addr];
  value = (value + 1) & U8_MAX;

  memory[addr] = value;
  cpu->P.Z = (value == 0);
  cpu->P.N = (value & 0x80) != 0;
}

#define INX() INX_c(&default_cpu)
void INX_c(cpu6502 *cpu){
  // Z N affected
  uint8_t value = cpu->X;
  value = (value + 1) & U8_MAX;

  cpu->X  = value;
  cpu->P.Z = (value == 0);
  cpu->P.N = (value & 0x80) != 0;
}

#define INY() INY_c(&default_cpu)
void INY_c(cpu6502 *cpu){
  // Z N affected
  uint8_t value = cpu->Y;
  value = (value + 1) & U8_MAX;

  cpu->Y  = value;
  cpu->P.Z = (value == 0);
  cpu->P.N = (value & 0x80) != 0;
}

#define JMP(addr) JMP_c(&default_cpu, addr)
void JMP_c(cpu6502 *cpu, uint16_t addr){
  cpu->PC = addr;
}

#define LDA(M) LDA_c(&default_cpu, M)
void LDA_c(cpu6502 *cpu, uint8_t M){
  // Z N affected
  cpu->A = M;
  cpu->P.Z = (cpu->A == 0);
  cpu->P.N = (cpu->A & 0x80) != 0;
}

#define LDX(M) LDX_c(&default_cpu, M)
void LDX_c(cpu6502 *cpu, uint8_t M){
  // Z N affected
  cpu->X = M;
  cpu->P.Z = (cpu->X == 0);
  cpu->P.N = (cpu->X & 0x80) != 0;
}

#define LDY(M) LDY(&default_cpu, M)
void LDY_c(cpu6502 *cpu, uint8_t M){
  // Z N affected
  cpu->A = M;
  cpu->P.Z = (cpu->Y == 0);
  cpu->P.N = (cpu->Y & 0x80) != 0;
}

#define ORA(M) ORA_c(&default_cpu, M)
void ORA_c(cpu6502 *cpu, uint8_t M){
  // Z N affected
  cpu->A = cpu->A | M;
  cpu->P.Z = (cpu->A == 0);
  cpu->P.N = (cpu->A & 0x80) != 0;
}

#define PHA() PHA_c(&default_cpu)
void PHA_c(cpu6502 *cpu){
  push_c(cpu, cpu->A);
}

#define PHP() PHP_c(&default_cpu)
void PHP_c(cpu6502 *cpu){
  // push(cpu, cpu->P); not sure how to do for now
}

#define PLA() PLA_c(&default_cpu)
uint8_t PLA_c(cpu6502 *cpu){
  return pull_c(cpu);
}

// TODO: IMPLEMENT PLP

#define SEC() SEC_c(&default_cpu)
void SEC_c(cpu6502 *cpu){
  cpu->P.C = 1;
}

#define SED() SED_c(&default_cpu)
void SED_c(cpu6502 *cpu){
  cpu->P.D = 1;
}

#define SEI() SEI_c(&default_cpu)
void SEI_c(cpu6502 *cpu){
  cpu->P.I = 1;
}

#define STA(addr) STA_c(&default_cpu, addr)
void STA_c(cpu6502 *cpu, uint8_t addr){
  memory[addr] = cpu->A;
}

#define STX(addr) STX_c(&default_cpu, addr)
void STX_c(cpu6502 *cpu, uint8_t addr){
  memory[addr] = cpu->X;
}

#define STY(addr) STY_c(&default_cpu, addr)
void STY_c(cpu6502 *cpu, uint8_t addr){
  memory[addr] = cpu->Y;
}

#define TAX() TAX_c(&default_cpu)
void TAX_c(cpu6502 *cpu){
  // Z N affected
  cpu->X = cpu->A;
  cpu->P.Z = (cpu->X == 0);
  cpu->P.N = (cpu->X & 0x80) != 0;
}

#define TAY() TAY_c(&default_cpu)
void TAY_c(cpu6502 *cpu){
  // Z N affected
  cpu->Y = cpu->A;
  cpu->P.Z = (cpu->Y == 0);
  cpu->P.N = (cpu->Y & 0x80) != 0;
}

#define TSX() TSX_c(&default_cpu)
void TSX_c(cpu6502 *cpu){
  // Z N affected
  cpu->X = cpu->SP;
  cpu->P.Z = (cpu->X == 0);
  cpu->P.N = (cpu->X & 0x80) != 0;
}

#define TXA() TXA_c(&default_cpu)
void TXA_c(cpu6502 *cpu){
  // Z N affected
  cpu->A = cpu->X;
  cpu->P.Z = (cpu->A == 0);
  cpu->P.N = (cpu->A & 0x80) != 0;
}

#define TXS() TXS_c(&default_cpu)
void TXS_c(cpu6502 *cpu){
  // Z N affected
  cpu->SP = cpu->X;
  cpu->P.Z = (cpu->SP == 0);
  cpu->P.N = (cpu->SP & 0x80) != 0;
}

#define TYA() TYA_c(&default_cpu)
void TYA_c(cpu6502 *cpu){
  // Z N affected
  cpu->A = cpu->Y;
  cpu->P.Z = (cpu->A == 0);
  cpu->P.N = (cpu->A & 0x80) != 0;
}

