#include <registers.hpp>
#include <interrupts.hpp>
#include <ram.hpp>
#include <cpu.hpp>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <byteswap.h>

void JUMP(uint16_t address, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    cpu->reg.write(IND_PC_NEXT, address);
    cpu->ticks += 12;
}

void LD_HL_DEC(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    cpu->ram.write8(cpu->reg.read(IND_HL), cpu->reg.read(ind));
    cpu->reg.write(IND_HL, cpu->reg.read(IND_HL) - 1);
    cpu->ticks += 8;
}

void LD_HL_INC(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    cpu->ram.write8(cpu->reg.read(IND_HL), cpu->reg.read(ind);
    cpu->reg.write(IND_HL, cpu->reg.read(IND_HL) + 1);
    cpu->ticks += 8;
}

void XOR(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(ind == IND_HL){
        cpu->reg.write(IND_A, (cpu->ram.read8(cpu->reg.read(IND_HL)) ^ (operand & 0xFF)));
        cpu->reg.F.bits.z = (cpu->reg.read(IND_A) == 0) ? true : false;
        cpu->ticks += 8;
    } else if(ind == IND_NONE){
        cpu->reg.write(IND_A, (cpu->ram.read8(operand) ^ (operand & 0xFF)));
        cpu->reg.F.bits.z = (cpu->reg.read(IND_A) == 0) ? true : false;
        cpu->ticks += 8;
    }
    cpu->reg.write(IND_A, (cpu->reg.read(ind) ^ (operand & 0xFF)));
    cpu->reg.F.bits.z = (cpu->reg.read(IND_A) == 0) ? true : false;
    cpu->ticks += 4;
}

void DEC(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(ind == IND_MEM_HL){
        cpu->ram.write8(cpu->ram.read8(cpu->reg.read(IND_HL)) - 1);
        cpu->reg.F.bits.z = (cpu->ram[(cpu->reg.read(IND_HL) - M_RAM_0)] == 0) ? true : false;
        cpu->reg.F.bits.h = ((cpu->ram[(cpu->reg.read(IND_HL) - M_RAM_0)] & 0x0F) == 0x0F);
        cpu->ticks += 12;
    } else if(ind == IND_BC || ind == IND_DE || ind == IND_HL || ind == IND_SP){
        cpu->reg.write(ind, cpu->reg.read(ind) - 1);
        cpu->reg.F.bits.z = (cpu->reg.read(ind) == 0) ? true : false;
        cpu->reg.F.bits.h = ((cpu->reg.read(ind) & 0x0FFF) == 0x0000);
        cpu->ticks += 8;
    } else {
        cpu->reg.write(ind, cpu->reg.read(ind) - 1);
        cpu->reg.F.bits.z = (cpu->reg.read(ind) == 0) ? true : false;
        cpu->reg.F.bits.h = ((cpu->reg.read(ind) & 0x0F) == 0x0F);
        cpu->ticks += 4;
    }
}

void INC(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(ind == IND_MEM_HL){ //8 bit indirected
        cpu->ram[(cpu->reg.read(IND_HL) - M_RAM_0)]++;
        cpu->reg.F.bits.z = (cpu->ram[(cpu->reg.read(IND_HL) - M_RAM_0)] == 0) ? true : false;
        cpu->reg.F.bits.h = ((cpu->ram[(cpu->reg.read(IND_HL) - M_RAM_0)] & 0x0F) == 0x00);
        cpu->ticks += 12;
    } else if(ind == IND_BC || ind == IND_DE || ind == IND_HL || ind == IND_SP){ //16 bit direct
        cpu->reg.write(ind, cpu->reg.read(ind) + 1);
        cpu->reg.F.bits.z = (cpu->reg.read(ind) == 0) ? true : false;
        cpu->reg.F.bits.h = ((cpu->reg.read(ind) & 0x0FFF) == 0x0000);
        cpu->ticks += 8;
    } else { //8 bit direct
        cpu->reg.write(ind, cpu->reg.read(ind) + 1);
        cpu->reg.F.bits.z = (cpu->reg.read(ind) == 0) ? true : false;
        cpu->reg.F.bits.h = ((cpu->reg.read(ind) & 0x0F) == 0x00);
        cpu->ticks += 4;
    }
}

void LD16(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(ind == IND_NONE){
        cpu->ram[operand - M_RAM_0] = cpu->reg.read(ind2);
        cpu->ticks += 16;
    } else{
        cpu->reg.write(ind, operand);
        cpu->ticks += 8;
    }
}

void LDA_8_IND(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(ind == IND_BC || ind == IND_DE || ind == IND_HL || ind == IND_SP){
        cpu->reg.write(IND_A, cpu->ram[cpu->reg.read(ind) - M_RAM_0]);
        cpu->ticks += 8;
    } else if(ind == IND_NONE){
        cpu->reg.write(IND_A, cpu->ram[operand- M_RAM_0]);
        cpu->ticks += 16;
    } else{
        cpu->reg.write(IND_A, cpu->reg.read(ind));
        cpu->ticks += 4;
    }
}

void LD8(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    cpu->reg.write(ind, (operand & 0xFF));
    cpu->ticks += 8;
}

void JRNZ(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(!cpu->reg.F.bits.z){
        cpu->reg.PC_next = cpu->reg.PC + (int8_t)((operand & 0xFF) + 2); //cast to signed int for hopefully subtraction
    printf("Jumping by %d bytes!\n", (int8_t)((operand & 0xFF) + 2));
    }
    cpu->ticks += 8;
}

void JPNC(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(!cpu->reg.F.bits.cy){
        cpu->reg.PC_next = cpu->reg.PC + (operand & 0xFF);
    }
    cpu->ticks += 8;
}

void JPNZ(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(!cpu->reg.F.bits.z){
        cpu->reg.PC_next = cpu->reg.PC + (operand & 0xFF);
    }
    cpu->ticks += 8;
}

void RR(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    uint8_t carry_old = cpu->reg.F.bits.cy;
    cpu->reg.F.bits.cy = (cpu->reg.read(ind) & 0x01);
    cpu->reg.write(ind, (cpu->reg.read(ind) >> 1) ^ carry_old << 8);
    cpu->ticks += 8;
}

void RL(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    uint8_t carry_old = cpu->reg.F.bits.cy;
    cpu->reg.F.bits.cy = (cpu->reg.read(ind) & 0x80);
    cpu->reg.write(ind, (cpu->reg.read(ind) << 1) ^ carry_old >> 8);
    cpu->ticks += 8;
}

void SH_R(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    cpu->reg.F.bits.cy = (cpu->reg.read(ind) & 0x01);
    cpu->reg.write(ind, (cpu->reg.read(ind) >> 1));
    cpu->ticks += 4;
}

void SH_L(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    cpu->reg.F.bits.cy = (cpu->reg.read(ind) & 0x80);
    cpu->reg.write(ind, (cpu->reg.read(ind) << 1));
    cpu->ticks += 4;
}

void OR(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(ind == IND_MEM_HL){
        cpu->reg.write(IND_A, (cpu->ram[(cpu->reg.read(IND_HL)) - M_RAM_0] | cpu->reg.read(IND_A)));
        cpu->reg.F.bits.z = (cpu->reg.read(IND_A) == 0) ? true : false;
        cpu->ticks += 8;
    }
    cpu->reg.write(IND_A, (cpu->reg.read(ind) | cpu->reg.read(IND_A)));
    cpu->reg.F.bits.z = (cpu->reg.read(IND_A) == 0) ? true : false;
    cpu->ticks += 4;
}

void LDMV(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(ind == IND_MEM_HL){
        cpu->ram[cpu->reg.read(IND_HL) - M_RAM_0] = cpu->reg.read(ind2);
        cpu->ticks += 8;
    }
    else{
        cpu->reg.write(ind, (cpu->reg.read(ind2)));
        cpu->ticks += 4;
    }
}

void LDDIR(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(ind == IND_NONE){
        cpu->reg.write(ind2, cpu->ram[(0xFF00 + (operand & 0xFF)) - M_RAM_0]);
        cpu->ticks += 12;
    } else if(ind2 == IND_NONE){
        cpu->ram[(0xFF00 + (operand & 0xFF)) - M_RAM_0] = cpu->reg.read(ind);
        cpu->ticks += 12;
    }
    else{
        cpu->ram[(0xFF00 + cpu->reg.read(ind)) - M_RAM_0] = cpu->reg.read(ind2);
        cpu->ticks + 8;
    }
}

void LD_DIR_16(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    cpu->ram[cpu->reg.read(ind) - M_RAM_0] = cpu->reg.read(ind2);
    cpu->ticks += 8;
}

void CP(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(ind == IND_MEM_HL){
        uint8_t tmp = cpu->reg.read(IND_A) - cpu->ram[(cpu->reg.read(IND_HL) - M_RAM_0)];
        cpu->reg.F.bits.z = (tmp == 0) ? true : false;
        cpu->reg.F.bits.h = (tmp & 0x0F) == 0x0F;
        cpu->reg.F.bits.cy = (cpu->reg.read(IND_A) < cpu->ram[(cpu->reg.read(IND_HL) - M_RAM_0)]) ? true : false;
        cpu->ticks += 8;
    } else if(ind == IND_NONE) {
        uint8_t tmp = cpu->reg.read(IND_A) - (operand & 0xFF);
        cpu->reg.F.bits.z = (tmp == 0) ? true : false;
        cpu->reg.F.bits.h = (tmp & 0x0F) == 0x0F;
        cpu->reg.F.bits.cy = (cpu->reg.read(IND_A) < cpu->reg.read(ind)) ? true : false;
        cpu->ticks += 8;
    } else {
        uint8_t tmp = cpu->reg.read(IND_A) - cpu->reg.read(ind);
        cpu->reg.F.bits.z = (tmp == 0) ? true : false;
        cpu->reg.F.bits.h = (tmp & 0x0F) == 0x0F;
        cpu->reg.F.bits.cy = (cpu->reg.read(IND_A) < cpu->reg.read(ind)) ? true : false;
        cpu->ticks += 4;
    }
}

void ADD16(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    cpu->reg.write(ind, cpu->reg.read(ind) + cpu->reg.read(ind2));
    cpu->reg.F.bits.n = false;
    cpu->reg.F.bits.h = (((cpu->reg.read(ind) & 0xF000) + (cpu->reg.read(ind2) & 0xF000)) & 0x1000) == 0x1000;
    cpu->reg.F.bits.cy = (cpu->reg.read(ind) > UINT16_MAX - cpu->reg.read(ind2)) ? true : false;
    cpu->ticks += 8;
}

void ADC8(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(ind == IND_MEM_HL){
        cpu->reg.write(IND_A, (cpu->reg.read(IND_A) + cpu->ram[(cpu->reg.read(IND_HL) - M_RAM_0)] + cpu->reg.F.bits.cy));
        cpu->reg.F.bits.cy = (cpu->reg.read(IND_A) > UINT16_MAX - cpu->ram[(cpu->reg.read(IND_HL) - M_RAM_0)]) ? true : false;
        cpu->ticks += 8;
    } else{
        cpu->reg.write(IND_A, (cpu->reg.read(IND_A) + cpu->reg.read(ind) + cpu->reg.F.bits.cy));
        cpu->reg.F.bits.cy = (cpu->reg.read(IND_A) > UINT16_MAX - cpu->reg.read(ind)) ? true : false;
        cpu->ticks += 4;
    }
    cpu->reg.F.bits.z = (cpu->reg.read(IND_A) == 0) ? true : false;
    cpu->reg.F.bits.h = ((cpu->reg.read(ind) & 0x0F) == 0x00);
    cpu->reg.F.bits.cy = (cpu->reg.read(IND_A) > UINT16_MAX - cpu->reg.read(ind)) ? true : false;
}

void ADD8(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(ind == IND_MEM_HL){
        cpu->reg.write(IND_A, (cpu->reg.read(IND_A) + cpu->ram[(cpu->reg.read(IND_HL) - M_RAM_0)]));
        cpu->reg.F.bits.cy = (cpu->reg.read(IND_A) > UINT16_MAX - cpu->ram[(cpu->reg.read(IND_HL) - M_RAM_0)]) ? true : false;
        cpu->ticks += 8;
    } else{
        cpu->reg.write(IND_A, (cpu->reg.read(IND_A) + cpu->reg.read(ind)));
        cpu->reg.F.bits.cy = (cpu->reg.read(IND_A) > UINT16_MAX - cpu->reg.read(ind)) ? true : false;
        cpu->ticks += 4;
    }
    cpu->reg.F.bits.z = (cpu->reg.read(IND_A) == 0) ? true : false;
    cpu->reg.F.bits.h = ((cpu->reg.read(ind) & 0x0F) == 0x00);
}

void SUB8(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(ind == IND_MEM_HL){
        cpu->reg.write(IND_A, (cpu->reg.read(IND_A) - cpu->ram[(cpu->reg.read(IND_HL) - M_RAM_0)]));
        cpu->reg.F.bits.cy = (cpu->reg.read(IND_A) > cpu->ram[(cpu->reg.read(IND_HL) - M_RAM_0)]) ? true : false;
        cpu->ticks += 8; 
    } else{
        cpu->reg.write(IND_A, (cpu->reg.read(IND_A) - cpu->reg.read(ind)));
        cpu->reg.F.bits.cy = (cpu->reg.read(IND_A) > cpu->reg.read(ind)) ? true : false;
        cpu->ticks += 4;
    }
    cpu->reg.F.bits.z = (cpu->reg.read(IND_A) == 0) ? true : false;
    cpu->reg.F.bits.h = ((cpu->reg.read(ind) & 0x0F) == 0x00);
}

void DI(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    printf("INTERRUPTS DISABLED\n");
    cpu->reg.interrupts_disabled_request = true;
    cpu->ticks += 4;
}

void EI(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    printf("INTERRUPTS ENABLED\n");
    cpu->reg.interrupts_disabled_request = false;
    cpu->ticks += 4;
}

void RST00(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    cpu->ram[cpu->reg.read(IND_SP) - M_RAM_0] = (cpu->reg.read(IND_PC) & 0xFF);
    cpu->ram[cpu->reg.read(IND_SP) + 1 - M_RAM_0] = (cpu->reg.read(IND_PC) & 0xFF00) >> 8;
    cpu->reg.write(IND_SP, cpu->reg.read(IND_SP) - 2);
    cpu->reg.PC_next = 0x0000;
    cpu->ticks += 32;
}

void RST08(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    cpu->ram[cpu->reg.read(IND_SP) - M_RAM_0] = (cpu->reg.read(IND_PC) & 0xFF);
    cpu->ram[cpu->reg.read(IND_SP) + 1 - M_RAM_0] = (cpu->reg.read(IND_PC) & 0xFF00) >> 8;
    cpu->reg.write(IND_SP, cpu->reg.read(IND_SP) - 2);
    cpu->reg.PC_next = 0x0008;
    cpu->ticks += 32;
}

void RST10(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    cpu->ram[cpu->reg.read(IND_SP) - M_RAM_0] = (cpu->reg.read(IND_PC) & 0xFF);
    cpu->ram[cpu->reg.read(IND_SP) + 1 - M_RAM_0] = (cpu->reg.read(IND_PC) & 0xFF00) >> 8;
    cpu->reg.write(IND_SP, cpu->reg.read(IND_SP) - 2);
    cpu->reg.PC_next = 0x0010;
    cpu->ticks += 32;
}

void RST18(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    cpu->ram[cpu->reg.read(IND_SP) - M_RAM_0] = (cpu->reg.read(IND_PC) & 0xFF);
    cpu->ram[cpu->reg.read(IND_SP) + 1 - M_RAM_0] = (cpu->reg.read(IND_PC) & 0xFF00) >> 8;
    cpu->reg.write(IND_SP, cpu->reg.read(IND_SP) - 2);
    cpu->reg.PC_next = 0x0018;
    cpu->ticks += 32;
}

void RST20(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    cpu->ram[cpu->reg.read(IND_SP) - M_RAM_0] = (cpu->reg.read(IND_PC) & 0xFF);
    cpu->ram[cpu->reg.read(IND_SP) + 1 - M_RAM_0] = (cpu->reg.read(IND_PC) & 0xFF00) >> 8;
    cpu->reg.write(IND_SP, cpu->reg.read(IND_SP) - 2);
    cpu->reg.PC_next = 0x0020;
    cpu->ticks += 32;
}

void RST28(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    cpu->ram[cpu->reg.read(IND_SP) - M_RAM_0] = (cpu->reg.read(IND_PC) & 0xFF);
    cpu->ram[cpu->reg.read(IND_SP) + 1 - M_RAM_0] = (cpu->reg.read(IND_PC) & 0xFF00) >> 8;
    cpu->reg.write(IND_SP, cpu->reg.read(IND_SP) - 2);
    cpu->reg.PC_next = 0x0028;
    cpu->ticks += 32;
}

void RST30(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    cpu->ram[cpu->reg.read(IND_SP) - M_RAM_0] = (cpu->reg.read(IND_PC) & 0xFF);
    cpu->ram[cpu->reg.read(IND_SP) + 1 - M_RAM_0] = (cpu->reg.read(IND_PC) & 0xFF00) >> 8;
    cpu->reg.write(IND_SP, cpu->reg.read(IND_SP) - 2);
    cpu->reg.PC_next = 0x0030;
    cpu->ticks += 32;
}

void RST38(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    cpu->ram[cpu->reg.read(IND_SP) - M_RAM_0] = (cpu->reg.read(IND_PC) & 0xFF);
    cpu->ram[cpu->reg.read(IND_SP) + 1 - M_RAM_0] = (cpu->reg.read(IND_PC) & 0xFF00) >> 8;
    cpu->reg.write(IND_SP, cpu->reg.read(IND_SP) - 2);
    cpu->reg.PC_next = 0x0038;
    cpu->ticks += 32;
}

void CALL(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    cpu->reg.write(IND_PC, cpu->reg.read(IND_PC) + 3); //move PC 3 forward (next instruction)
    cpu->ram[cpu->reg.read(IND_SP) - M_RAM_0] = (cpu->reg.read(IND_PC) & 0xFF);
    cpu->ram[cpu->reg.read(IND_SP) + 1 - M_RAM_0] = (cpu->reg.read(IND_PC) & 0xFF00) >> 8;
    cpu->reg.write(IND_SP, cpu->reg.read(IND_SP) - 2); //move SP forward
    cpu->reg.PC_next = operand;
    cpu->ticks += 12;
}

void RET(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    cpu->reg.PC_next = cpu->ram[cpu->reg.read(IND_SP) - M_RAM_0];
    cpu->reg.write(IND_SP, cpu->reg.read(IND_SP)-2);
    cpu->ticks += 8;
}

void NOP(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    cpu->ticks += 4;
}

opcode opcodes[256] = {
    {"NOP",               0, IND_NONE      , IND_NONE  , NOP}, //0x00
    {"LD BC,16b",         2, IND_BC        , IND_NONE  , LD16},
    {"LD (BC), A",        0, IND_BC        , IND_A     , LD_DIR_16},
    {"INC BC",            0, IND_BC        , IND_NONE  , INC},
    {"INC B",             0, IND_B         , IND_NONE  , INC},
    {"DEC B",             0, IND_B         , IND_NONE  , DEC},
    {"LD B,8b",           1, IND_B         , IND_NONE  , LD8 },
    {"RLCA",              0, IND_A         , IND_NONE  , SH_L},
    {"LD SP,16b",         2, IND_SP        , IND_NONE  , LD16},
    {"ADD HL,BC",         0, IND_HL        , IND_BC    , ADD16},
    {"LD A,(BC)",         0, IND_NONE      , IND_NONE  , NULL},
    {"DEC BC",            0, IND_BC        , IND_NONE  , DEC},
    {"INC C",             0, IND_C         , IND_NONE  , INC},
    {"DEC C",             0, IND_C         , IND_NONE  , DEC},
    {"LD C,8b",           1, IND_C         , IND_NONE  , LD8},
    {"RRCA",              0, IND_A         , IND_NONE  , SH_L},
    {"STOP 0",            1, IND_NONE      , IND_NONE  , NULL}, //0x10
    {"LD DE,16b",         2, IND_DE        , IND_NONE  , LD16},
    {"LD DE, A",          0, IND_NONE      , IND_NONE  , NULL},
    {"INC DE",            0, IND_DE        , IND_NONE  , INC},
    {"INC D",             0, IND_D         , IND_NONE  , INC},
    {"DEC D",             0, IND_D         , IND_NONE  , DEC},
    {"LD D 8b",           1, IND_D         , IND_NONE  , LD8},
    {"RLA",               0, IND_A         , IND_NONE  , RL},
    {"JR 8b",             1, IND_NONE      , IND_NONE  , NULL},
    {"ADD HL,DE",         0, IND_HL        , IND_DE    , ADD16},
    {"LD A, DE",          0, IND_NONE      , IND_NONE  , NULL},
    {"DEC DE",            0, IND_DE        , IND_NONE  , DEC},
    {"INC E",             0, IND_E         , IND_NONE  , INC},
    {"DEC E",             0, IND_E         , IND_NONE  , DEC},
    {"LD E,8b",           1, IND_E         , IND_NONE  , LD8},
    {"RRA",               0, IND_A         , IND_NONE  , RR},
    {"JR NZ,8b",          1, IND_NONE      , IND_NONE  , JRNZ}, //0x20
    {"LD HL,16b",         2, IND_HL        , IND_NONE  , LD16},
    {"LD HL+,A",          0, IND_NONE      , IND_NONE  , NULL},
    {"INC HL",            0, IND_HL        , IND_NONE  , INC},
    {"INC H",             0, IND_H         , IND_NONE  , INC},
    {"DEC H",             0, IND_H         , IND_NONE  , DEC},
    {"LD H,8b",           1, IND_H         , IND_NONE  , LD8},
    {"DAA",               0, IND_NONE      , IND_NONE  , NULL},
    {"JR Z,8b",           1, IND_NONE      , IND_NONE  , NULL},
    {"ADD HL,HL",         0, IND_HL        , IND_HL    , ADD16},
    {"LD A,HL+",          0, IND_A         , IND_NONE  , LD_HL_INC},
    {"DEC HL",            0, IND_HL        , IND_NONE  , DEC}, //indirected but register-local
    {"INC L",             0, IND_L         , IND_NONE  , INC},
    {"DEC L",             0, IND_L         , IND_NONE  , DEC},
    {"LD L,8b",           1, IND_L         , IND_NONE  , LD8},
    {"CPL",               0, IND_L         , IND_NONE  , NULL},
    {"JR NC,8b",          1, IND_NONE      , IND_NONE  , NULL}, //0x30
    {"LD SP,16b",         2, IND_SP        , IND_NONE  , LD16},
    {"LD HL-,A",          0, IND_A         , IND_NONE  , LD_HL_DEC},
    {"INC SP",            0, IND_SP        , IND_NONE  , INC},
    {"INC HL",            0, IND_HL        , IND_NONE  , INC},
    {"DEC(HL)",           0, IND_MEM_HL    , IND_NONE  , NULL}, //indirected and RAM
    {"LD HL,8b",          1, IND_HL        , IND_NONE  , LD8},
    {"SCF",               0, IND_NONE      , IND_NONE  , NULL},
    {"JR C,8b",           1, IND_NONE      , IND_NONE  , NULL},
    {"ADD HL,SP",         0, IND_HL        , IND_SP    , ADD16},
    {"LD A,HL-",          0, IND_NONE      , IND_NONE  , NULL},
    {"DEC SP",            0, IND_NONE      , IND_NONE  , NULL},
    {"INC A",             0, IND_A         , IND_NONE  , INC},
    {"DEC A",             0, IND_NONE      , IND_NONE  , NULL},
    {"LD A,8b",           1, IND_A         , IND_NONE  , LD8},
    {"CCF",               0, IND_NONE      , IND_NONE  , NULL},
    {"LD B,B",            0, IND_B         , IND_B     , LDMV}, //0x40
    {"LD B,C",            0, IND_B         , IND_C     , LDMV},
    {"LD B,D",            0, IND_B         , IND_D     , LDMV},
    {"LD B,E",            0, IND_B         , IND_E     , LDMV},
    {"LD B,H",            0, IND_B         , IND_H     , LDMV},
    {"LD B,L",            0, IND_B         , IND_L     , LDMV},
    {"LD B,HL",           0, IND_B         , IND_MEM_HL, LDMV},
    {"LD B,A",            0, IND_B         , IND_A     , LDMV},
    {"LD C,B",            0, IND_C         , IND_B     , LDMV},
    {"LD C,C",            0, IND_C         , IND_C     , LDMV},
    {"LD C,D",            0, IND_C         , IND_D     , LDMV},
    {"LD C,E",            0, IND_C         , IND_E     , LDMV},
    {"LD C,H",            0, IND_C         , IND_H     , LDMV},
    {"LD C,L",            0, IND_C         , IND_L     , LDMV},
    {"LD C,HL",           0, IND_C         , IND_MEM_HL, LDMV},
    {"LD C,A",            0, IND_C         , IND_A     , LDMV},
    {"LD D,B",            0, IND_D         , IND_B     , LDMV}, //0x50
    {"LD D,C",            0, IND_D         , IND_C     , LDMV},
    {"LD D,D",            0, IND_D         , IND_D     , LDMV},
    {"LD D,E",            0, IND_D         , IND_E     , LDMV},
    {"LD D,H",            0, IND_D         , IND_H     , LDMV},
    {"LD D,L",            0, IND_D         , IND_L     , LDMV},
    {"LD D,HL",           0, IND_D         , IND_MEM_HL, LDMV},
    {"LD D,A",            0, IND_D         , IND_A     , LDMV},
    {"LD E,B",            0, IND_E         , IND_B     , LDMV},
    {"LD E,C",            0, IND_E         , IND_C     , LDMV},
    {"LD E,D",            0, IND_E         , IND_D     , LDMV},
    {"LD E,E",            0, IND_E         , IND_E     , LDMV},
    {"LD E,H",            0, IND_E         , IND_H     , LDMV},
    {"LD E,L",            0, IND_E         , IND_L     , LDMV},
    {"LD E,HL",           0, IND_E         , IND_MEM_HL, LDMV},
    {"LD E,A",            0, IND_E         , IND_A     , LDMV},
    {"LD H,B",            0, IND_H         , IND_B     , LDMV}, //0x60
    {"LD H,C",            0, IND_H         , IND_C     , LDMV},
    {"LD H,D",            0, IND_H         , IND_D     , LDMV},
    {"LD H,E",            0, IND_H         , IND_E     , LDMV},
    {"LD H,H",            0, IND_H         , IND_H     , LDMV},
    {"LD H,L",            0, IND_H         , IND_L     , LDMV},
    {"LD H,HL",           0, IND_H         , IND_MEM_HL, LDMV},
    {"LD H,A",            0, IND_H         , IND_A     , LDMV},
    {"LD L,B",            0, IND_L         , IND_B     , LDMV},
    {"LD L,C",            0, IND_L         , IND_C     , LDMV},
    {"LD L,D",            0, IND_L         , IND_D     , LDMV},
    {"LD L,E",            0, IND_L         , IND_E     , LDMV},
    {"LD L,H",            0, IND_L         , IND_H     , LDMV},
    {"LD L,L",            0, IND_L         , IND_L     , LDMV},
    {"LD L,HL",           0, IND_L         , IND_MEM_HL, LDMV},
    {"LD L,A",            0, IND_L         , IND_A     , LDMV},
    {"LD HL,B",           0, IND_MEM_HL    , IND_B     , LDMV}, //0x70
    {"LD HL,C",           0, IND_MEM_HL    , IND_C     , LDMV},
    {"LD HL,D",           0, IND_MEM_HL    , IND_D     , LDMV},
    {"LD HL,E",           0, IND_MEM_HL    , IND_E     , LDMV},
    {"LD HL,H",           0, IND_MEM_HL    , IND_H     , LDMV},
    {"LD HL,L",           0, IND_MEM_HL    , IND_L     , LDMV},
    {"HALT",              0, IND_NONE      , IND_NONE  , NULL},
    {"LD HL,A",           0, IND_MEM_HL    , IND_A     , LDMV},
    {"LD A,B",            0, IND_A         , IND_B     , LDMV},
    {"LD A,C",            0, IND_A         , IND_C     , LDMV},
    {"LD A,D",            0, IND_A         , IND_D     , LDMV},
    {"LD A,E",            0, IND_A         , IND_E     , LDMV},
    {"LD A,H",            0, IND_A         , IND_H     , LDMV},
    {"LD A,L",            0, IND_A         , IND_L     , LDMV},
    {"LD A,HL",           0, IND_A         , IND_HL    , LDMV},
    {"LD A,A",            0, IND_A         , IND_A     , LDMV},
    {"ADD A,B",           0, IND_B         , IND_NONE  , ADD8}, //0x80
    {"ADD A,C",           0, IND_C         , IND_NONE  , ADD8},
    {"ADD A,D",           0, IND_D         , IND_NONE  , ADD8},
    {"ADD A,E",           0, IND_E         , IND_NONE  , ADD8},
    {"ADD A,H",           0, IND_H         , IND_NONE  , ADD8},
    {"ADD A,L",           0, IND_L         , IND_NONE  , ADD8},
    {"ADD A,HL",          0, IND_MEM_HL    , IND_NONE  , ADD8},
    {"ADD A,A",           0, IND_A         , IND_NONE  , ADD8},
    {"ADC A,B",           0, IND_B         , IND_NONE  , ADC8},
    {"ADC A,C",           0, IND_C         , IND_NONE  , ADC8},
    {"ADC A,D",           0, IND_D         , IND_NONE  , ADC8},
    {"ADC A,E",           0, IND_E         , IND_NONE  , ADC8},
    {"ADC A,H",           0, IND_H         , IND_NONE  , ADC8},
    {"ADC A,L",           0, IND_L         , IND_NONE  , ADC8},
    {"ADC A,HL",          0, IND_MEM_HL    , IND_NONE  , ADC8},
    {"ADC A,A",           0, IND_A         , IND_NONE  , ADC8},
    {"SUB B",             0, IND_B         , IND_NONE  , SUB8}, //0x90
    {"SUB C",             0, IND_C         , IND_NONE  , SUB8},
    {"SUB D",             0, IND_D         , IND_NONE  , SUB8},
    {"SUB E",             0, IND_E         , IND_NONE  , SUB8},
    {"SUB H",             0, IND_H         , IND_NONE  , SUB8},
    {"SUB L",             0, IND_L         , IND_NONE  , SUB8},
    {"SUB HL",            0, IND_MEM_HL    , IND_NONE  , SUB8},
    {"SUB A",             0, IND_A         , IND_NONE  , SUB8},
    {"SBC A,B",           0, IND_NONE      , IND_NONE  , NULL},
    {"SBC A,C",           0, IND_NONE      , IND_NONE  , NULL},
    {"SBC A,D",           0, IND_NONE      , IND_NONE  , NULL},
    {"SBC A,E",           0, IND_NONE      , IND_NONE  , NULL},
    {"SBC A,H",           0, IND_NONE      , IND_NONE  , NULL},
    {"SBC A,L",           0, IND_NONE      , IND_NONE  , NULL},
    {"SBC A,HL",          0, IND_NONE      , IND_NONE  , NULL},
    {"SBC A,A",           0, IND_NONE      , IND_NONE  , NULL},
    {"AND B",             0, IND_NONE      , IND_NONE  , NULL}, //0xA0
    {"AND C",             0, IND_NONE      , IND_NONE  , NULL},
    {"AND D",             0, IND_NONE      , IND_NONE  , NULL},
    {"AND E",             0, IND_NONE      , IND_NONE  , NULL},
    {"AND H",             0, IND_NONE      , IND_NONE  , NULL},
    {"AND L",             0, IND_NONE      , IND_NONE  , NULL},
    {"AND HL",            0, IND_NONE      , IND_NONE  , NULL},
    {"AND A",             0, IND_NONE      , IND_NONE  , NULL},
    {"XOR B",             0, IND_B         , IND_NONE  , XOR},
    {"XOR C",             0, IND_C         , IND_NONE  , XOR},
    {"XOR D",             0, IND_D         , IND_NONE  , XOR},
    {"XOR E",             0, IND_E         , IND_NONE  , XOR},
    {"XOR H",             0, IND_H         , IND_NONE  , XOR},
    {"XOR L",             0, IND_L         , IND_NONE  , XOR},
    {"XOR HL",            0, IND_HL        , IND_NONE  , XOR},
    {"XOR A",             0, IND_A         , IND_NONE  , XOR},
    {"OR B",              0, IND_B         , IND_NONE  , OR}, //0xB0
    {"OR C",              0, IND_C         , IND_NONE  , OR},
    {"OR D",              0, IND_D         , IND_NONE  , OR},
    {"OR E",              0, IND_E         , IND_NONE  , OR},
    {"OR H",              0, IND_H         , IND_NONE  , OR},
    {"OR L",              0, IND_L         , IND_NONE  , OR},
    {"OR HL",             0, IND_MEM_HL    , IND_NONE  , OR},
    {"OR A",              0, IND_A         , IND_NONE  , OR},
    {"CP B",              0, IND_B         , IND_NONE  , CP},
    {"CP C",              0, IND_C         , IND_NONE  , CP},
    {"CP D",              0, IND_D         , IND_NONE  , CP},
    {"CP E",              0, IND_E         , IND_NONE  , CP},
    {"CP H",              0, IND_H         , IND_NONE  , CP},
    {"CP L",              0, IND_L         , IND_NONE  , CP},
    {"CP HL",             0, IND_MEM_HL    , IND_NONE  , CP},
    {"CP A",              0, IND_A         , IND_NONE  , CP},
    {"RET NZ",            0, IND_NONE      , IND_NONE  , NULL}, //0xC0
    {"POP BC",            0, IND_NONE      , IND_NONE  , NULL},
    {"JP NZ,16b",         2, IND_NONE      , IND_NONE  , NULL},
    {"JP 16b",            2, IND_NONE      , IND_NONE  , JUMP},
    {"CALL NZ,16b",       2, IND_NONE      , IND_NONE  , NULL},
    {"PUSH BC",           0, IND_NONE      , IND_NONE  , NULL},
    {"ADD A,8b",          1, IND_NONE      , IND_NONE  , NULL},
    {"RST 00",            0, IND_NONE      , IND_NONE  , RST00},
    {"RET Z",             0, IND_NONE      , IND_NONE  , NULL},
    {"RET",               0, IND_NONE      , IND_NONE  , RET},
    {"JP Z,16b",          0, IND_NONE      , IND_NONE  , NULL},
    {"PREFIX CB",         0, IND_NONE      , IND_NONE  , NULL},
    {"CALL Z,16b",        0, IND_NONE      , IND_NONE  , NULL},
    {"CALL 16b",          2, IND_NONE      , IND_NONE  , CALL},
    {"ADC A,8b",          0, IND_NONE      , IND_NONE  , NULL},
    {"RST 08",            0, IND_NONE      , IND_NONE  , RST08},
    {"RET NC",            0, IND_NONE      , IND_NONE  , NULL}, //0xD0
    {"POP DE",            0, IND_NONE      , IND_NONE  , NULL},
    {"JP NZ,16b",         2, IND_NONE      , IND_NONE  , JPNZ},
    {"NULL INSTR",        0, IND_NONE      , IND_NONE  , NULL},
    {"CALL NC,16b",       2, IND_NONE      , IND_NONE  , NULL},
    {"PUSH DE",           0, IND_DE        , IND_NONE  , NULL},
    {"SUB ?????,8b",      1, IND_NONE      , IND_NONE  , NULL},
    {"RST 10h",           0, IND_NONE      , IND_NONE  , RST10},
    {"RET C",             0, IND_NONE      , IND_NONE  , NULL},
    {"RET I",             0, IND_NONE      , IND_NONE  , NULL},
    {"JP C,16b",          2, IND_NONE      , IND_NONE  , NULL},
    {"NULL INSTR",        0, IND_NONE      , IND_NONE  , NULL},
    {"CALL C,16b",        2, IND_NONE      , IND_NONE  , NULL},
    {"NULL INSTR",        0, IND_NONE      , IND_NONE  , NULL},
    {"SBC A,8b",          1, IND_NONE      , IND_NONE  , NULL},
    {"RST 18h",           0, IND_NONE      , IND_NONE  , RST18},
    {"LDH 8b, A",         1, IND_A         , IND_NONE  , LDDIR}, //0xE0
    {"POP HL",            0, IND_A         , IND_NONE  , NULL},
    {"LD (C), A",         0, IND_C         , IND_A     , LDDIR},
    {"NULL INSTR",        0, IND_NONE      , IND_NONE  , NULL},
    {"NULL INSTR",        0, IND_NONE      , IND_NONE  , NULL},
    {"PUSH HL",           0, IND_NONE      , IND_NONE  , NULL},
    {"AND 8b",            1, IND_NONE      , IND_NONE  , NULL},
    {"RST 20h",           0, IND_NONE      , IND_NONE  , RST20},
    {"ADD SP, 8b",        1, IND_NONE      , IND_NONE  , NULL},
    {"JP (HL)",           0, IND_NONE      , IND_NONE  , NULL},
    {"LD 16b, A",         2, IND_NONE      , IND_A     , LD16},
    {"NULL INSTR",        0, IND_NONE      , IND_NONE  , NULL},
    {"NULL INSTR",        0, IND_NONE      , IND_NONE  , NULL},
    {"NULL INSTR",        0, IND_NONE      , IND_NONE  , NULL},
    {"XOR 8b",            1, IND_NONE      , IND_NONE  , NULL},
    {"RST 28h",           0, IND_NONE      , IND_NONE  , RST28},
    {"LDH A,(8b)",        1, IND_NONE      , IND_A     , LDDIR}, //0xF0
    {"POP AF",            0, IND_NONE      , IND_NONE  , NULL},
    {"LD A,(C)",          1, IND_NONE      , IND_NONE  , NULL},
    {"DI",                0, IND_NONE      , IND_NONE  , DI},
    {"NULL INSTR",        0, IND_NONE      , IND_NONE  , NULL},
    {"NULL INSTR",        0, IND_NONE      , IND_NONE  , NULL},
    {"NULL INSTR",        0, IND_NONE      , IND_NONE  , NULL},
    {"NULL INSTR",        0, IND_NONE      , IND_NONE  , NULL},
    {"NULL INSTR",        0, IND_NONE      , IND_NONE  , NULL},
    {"NULL INSTR",        0, IND_NONE      , IND_NONE  , NULL},
    {"NULL INSTR",        0, IND_NONE      , IND_NONE  , NULL},
    {"NULL INSTR",        0, IND_NONE      , IND_NONE  , NULL},
    {"NULL INSTR",        0, IND_NONE      , IND_NONE  , NULL},
    {"NULL INSTR",        0, IND_NONE      , IND_NONE  , NULL},
    {"CP 8b",             1, IND_NONE      , IND_NONE  , CP  },
    {"RST 38h",           0, IND_NONE      , IND_NONE  , RST38},
};

cpu::cpu(uint8_t* p_rom_input){
    p_rom = p_rom_input;
    reg.PC = 0;
    ticks = 0;
    ram.set_interrupt_backend(&interrupts);
}

uint16_t find_16_from_8(uint8_t* array, int ptr){
    uint8_t low = array[ptr];
    uint8_t high = array[ptr+1];
    uint16_t combined = (uint16_t)high;
    combined = combined << 8;
    combined = combined + low;
    return combined;
}

uint16_t cpu::get_operand(opcode ctx){
    if(ctx.operand_length == 1){
        return(p_rom[reg.PC+1]);
    } else if(ctx.operand_length == 2){
        return(find_16_from_8(p_rom, reg.PC+1));
    } else{
        return(0);
    }
}

int cpu::execute_opcode(){
    opcode ctx = opcodes[p_rom[reg.PC]];
    uint16_t operand = get_operand(ctx);
    printf("Ticks: %d\n", (int)ticks);
    printf("PC: 0x%04X [0x%02X]:(%s) - {0x%04X}\n",reg.PC, p_rom[reg.PC], ctx.str_name, operand);

    if(ctx.opcode_pointer == NULL){
        printf("instruction not implemented :(\n");
        return(-1);
    } else{
        reg.PC_next = reg.PC + ctx.operand_length + 1;
        ctx.opcode_pointer(operand,this, ctx.reg_ind, ctx.reg_ind_two);
        reg.PC_step();
        return 0;
    }
}

void cpu::dump_registers(){
    printf("Register dump\n");
    printf("A: 0x%02X F: 0x%02X ", reg.A, reg.F.byte);
    printf("B: 0x%02X C: 0x%02X\n", reg.B, reg.C);
    printf("D: 0x%02X E: 0x%02X ", reg.D, reg.E);
    printf("H: 0x%02X L: 0x%02X\n", reg.H, reg.L);
    printf("PC: 0x%04X ", reg.PC);
    printf("SP: 0x%04X\n", reg.SP);
    printf("FLAGS:\n z; %d | n: %d | h: %d | cy: %d \n\n",
    reg.F.bits.z, reg.F.bits.n, reg.F.bits.h, reg.F.bits.cy);
}