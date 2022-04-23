#include <cpu.hpp>
#include <registers.hpp>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <byteswap.h>

void jump(uint16_t address, registers* p_reg, uint8_t* p_ram, reg_ind_t ind, reg_ind_t ind2){
    p_reg->write(REG_IND_PC_NEXT, address);
}

void LD_HL_DEC(uint16_t operand, registers* p_reg, uint8_t* p_ram, reg_ind_t ind, reg_ind_t ind2){
    p_ram[(p_reg->read(REG_IND_HL) - MEM_RAM_START)] = p_reg->read(ind);
}

void XOR(uint16_t operand, registers* p_reg, uint8_t* p_ram, reg_ind_t ind, reg_ind_t ind2){
    p_reg->write(REG_IND_A, (p_reg->read(ind) ^ (operand & 0xFF)));
    p_reg->F.bits.z = (p_reg->read(REG_IND_A) == 0) ? true : false;
}

void DEC(uint16_t operand, registers* p_reg, uint8_t* p_ram, reg_ind_t ind, reg_ind_t ind2){
    if(ind == REG_IND_MEM_HL){
        p_ram[(p_reg->read(REG_IND_HL) - MEM_RAM_START)]--;
        p_reg->F.bits.z = (p_ram[(p_reg->read(REG_IND_HL) - MEM_RAM_START)] == 0) ? true : false;
        p_reg->F.bits.h = ((p_ram[(p_reg->read(REG_IND_HL) - MEM_RAM_START)] & 0x0F) == 0x0F);
    } else {
        p_reg->write(ind, p_reg->read(ind) - 1);
        p_reg->F.bits.z = (p_reg->read(ind) == 0) ? true : false;
        p_reg->F.bits.h = ((p_reg->read(ind) & 0x0F) == 0x0F);
    }
}

void INC(uint16_t operand, registers* p_reg, uint8_t* p_ram, reg_ind_t ind, reg_ind_t ind2){
    if(ind == REG_IND_MEM_HL){
        p_ram[(p_reg->read(REG_IND_HL) - MEM_RAM_START)]++;
        p_reg->F.bits.z = (p_ram[(p_reg->read(REG_IND_HL) - MEM_RAM_START)] == 0) ? true : false;
        p_reg->F.bits.h = ((p_ram[(p_reg->read(REG_IND_HL) - MEM_RAM_START)] & 0x0F) == 0x00);
    } else {
        p_reg->write(ind, p_reg->read(ind) + 1);
        p_reg->F.bits.z = (p_reg->read(ind) == 0) ? true : false;
        p_reg->F.bits.h = ((p_reg->read(ind) & 0x0F) == 0x00);
    }
}

void LD16(uint16_t operand, registers* p_reg, uint8_t* p_ram, reg_ind_t ind, reg_ind_t ind2){
    p_reg->write(ind, operand);
}

void LDA_8_IND(uint16_t operand, registers* p_reg, uint8_t* p_ram, reg_ind_t ind, reg_ind_t ind2){
    if(ind == REG_IND_BC){
        p_reg->write(REG_IND_A, p_ram[(p_reg->BC) - MEM_RAM_START]);
    } else if(ind == REG_IND_DE){
        p_reg->write(REG_IND_A, p_ram[(p_reg->DE) - MEM_RAM_START]);
    } else if(ind == REG_IND_HL){
        p_reg->write(REG_IND_A, p_ram[(p_reg->HL) - MEM_RAM_START]);
    } else if(ind == REG_IND_NONE){
        p_reg->write(REG_IND_A, p_ram[operand- MEM_RAM_START]);
    } else{
        p_reg->write(REG_IND_A, p_reg->read(ind));
    }
}

void LD8(uint16_t operand, registers* p_reg, uint8_t* p_ram, reg_ind_t ind, reg_ind_t ind2){
    p_reg->write(ind, operand);
}

void JRNZ(uint16_t operand, registers* p_reg, uint8_t* p_ram, reg_ind_t ind, reg_ind_t ind2){
    if(!p_reg->F.bits.z){
        p_reg->PC_next = p_reg->PC + (operand & 0xFF);
    }
}

void JPNC(uint16_t operand, registers* p_reg, uint8_t* p_ram, reg_ind_t ind, reg_ind_t ind2){
    if(!p_reg->F.bits.cy){
        p_reg->PC_next = p_reg->PC + (operand & 0xFF);
    }
}

void JPNZ(uint16_t operand, registers* p_reg, uint8_t* p_ram, reg_ind_t ind, reg_ind_t ind2){
    if(!p_reg->F.bits.z){
        p_reg->PC_next = p_reg->PC + (operand & 0xFF);
    }
}

void RR(uint16_t operand, registers* p_reg, uint8_t* p_ram, reg_ind_t ind, reg_ind_t ind2){
    uint8_t carry_old = p_reg->F.bits.cy;
    p_reg->F.bits.cy = (p_reg->read(ind) & 0x01);
    p_reg->write(ind, (p_reg->read(ind) >> 1) ^ carry_old << 8);
}

void RL(uint16_t operand, registers* p_reg, uint8_t* p_ram, reg_ind_t ind, reg_ind_t ind2){
    uint8_t carry_old = p_reg->F.bits.cy;
    p_reg->F.bits.cy = (p_reg->read(ind) & 0x80);
    p_reg->write(ind, (p_reg->read(ind) << 1) ^ carry_old >> 8);
}

void SH_R(uint16_t operand, registers* p_reg, uint8_t* p_ram, reg_ind_t ind, reg_ind_t ind2){
    p_reg->F.bits.cy = (p_reg->read(ind) & 0x01);
    p_reg->write(ind, (p_reg->read(ind) >> 1));
}

void SH_L(uint16_t operand, registers* p_reg, uint8_t* p_ram, reg_ind_t ind, reg_ind_t ind2){
    p_reg->F.bits.cy = (p_reg->read(ind) & 0x80);
    p_reg->write(ind, (p_reg->read(ind) << 1));
}

void OR(uint16_t operand, registers* p_reg, uint8_t* p_ram, reg_ind_t ind, reg_ind_t ind2){
    if(ind == REG_IND_MEM_HL){
        p_reg->write(REG_IND_A, (p_ram[(p_reg->read(REG_IND_HL)) - MEM_RAM_START] | p_reg->read(REG_IND_A)));
        p_reg->F.bits.z = (p_reg->read(REG_IND_A) == 0) ? true : false;
    }
    p_reg->write(REG_IND_A, (p_reg->read(ind) | p_reg->read(REG_IND_A)));
    p_reg->F.bits.z = (p_reg->read(REG_IND_A) == 0) ? true : false;
}

void LDMV(uint16_t operand, registers* p_reg, uint8_t* p_ram, reg_ind_t ind, reg_ind_t ind2){
    if(ind == REG_IND_MEM_HL){
        p_ram[p_reg->read(REG_IND_HL) - MEM_RAM_START] = p_reg->read(ind2);
    }
    else if(ind2 == REG_IND_MEM_HL){
        p_reg->write(ind2, p_ram[p_reg->read(REG_IND_HL) - MEM_RAM_START]);
    }
    else{
        p_reg->write(ind, (p_reg->read(ind2)));
    }
}

void CP(uint16_t operand, registers* p_reg, uint8_t* p_ram, reg_ind_t ind, reg_ind_t ind2){
    if(ind == REG_IND_MEM_HL){
        uint8_t tmp = p_reg->read(REG_IND_A) - p_ram[(p_reg->read(REG_IND_HL) - MEM_RAM_START)];
        p_reg->F.bits.z = (tmp == 0) ? true : false;
        p_reg->F.bits.h = (tmp & 0x0F) == 0x0F;
        p_reg->F.bits.cy = (p_reg->read(REG_IND_A) < p_ram[(p_reg->read(REG_IND_HL) - MEM_RAM_START)]) ? true : false;
    } else {
        uint8_t tmp = p_reg->read(REG_IND_A) - p_reg->read(ind);
        p_reg->F.bits.z = (tmp == 0) ? true : false;
        p_reg->F.bits.h = (tmp & 0x0F) == 0x0F;
        p_reg->F.bits.cy = (p_reg->read(REG_IND_A) < p_reg->read(ind)) ? true : false;
    }
}

void ADD16(uint16_t operand, registers* p_reg, uint8_t* p_ram, reg_ind_t ind, reg_ind_t ind2){
    p_reg->write(ind, p_reg->read(ind) + p_reg->read(ind2));
    p_reg->F.bits.n = false;
    p_reg->F.bits.h = (((p_reg->read(ind) & 0xF000) + (p_reg->read(ind2) & 0xF000)) & 0x1000) == 0x1000;
    p_reg->F.bits.cy = (p_reg->read(ind) > UINT16_MAX - p_reg->read(ind2)) ? true : false;
}

void ADC8(uint16_t operand, registers* p_reg, uint8_t* p_ram, reg_ind_t ind, reg_ind_t ind2){
    if(ind == REG_IND_MEM_HL){
        //enough for a night lmao
    }
}


opcode opcodes[256] = {
    {"NOP",               0, REG_IND_NONE      , REG_IND_NONE  , NULL}, //0x00
    {"LD BC,16b",         2, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"LD (BC), A",        0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"INC BC",            0, REG_IND_BC        , REG_IND_NONE  , INC},
    {"INC B",             0, REG_IND_B         , REG_IND_NONE  , INC},
    {"DEC B",             0, REG_IND_B         , REG_IND_NONE  , DEC},
    {"LD B,8b",           1, REG_IND_B         , REG_IND_NONE  , LD8 },
    {"RLCA",              0, REG_IND_A         , REG_IND_NONE  , SH_L},
    {"LD SP,16b",         2, REG_IND_SP        , REG_IND_NONE  , LD16},
    {"ADD HL,BC",         0, REG_IND_HL        , REG_IND_BC    , ADD16},
    {"LD A,(BC)",         0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"DEC BC",            0, REG_IND_BC        , REG_IND_NONE  , DEC},
    {"INC C",             0, REG_IND_C         , REG_IND_NONE  , INC},
    {"DEC C",             0, REG_IND_C         , REG_IND_NONE  , NULL},
    {"LD C,8b",           1, REG_IND_C         , REG_IND_NONE  , LD8},
    {"RRCA",              0, REG_IND_A         , REG_IND_NONE  , SH_L}, //0x10
    {"STOP 0",            1, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"LD DE,16b",         2, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"LD DE, A",          0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"INC DE",            0, REG_IND_DE        , REG_IND_NONE  , INC},
    {"INC D",             0, REG_IND_D         , REG_IND_NONE  , INC},
    {"DEC D",             0, REG_IND_D         , REG_IND_NONE  , DEC},
    {"LD D 8b",           1, REG_IND_D         , REG_IND_NONE  , LD8},
    {"RLA",               0, REG_IND_A         , REG_IND_NONE  , RL},
    {"JR 8b",             1, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"ADD HL,DE",         0, REG_IND_HL        , REG_IND_DE    , ADD16},
    {"LD A, DE",          0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"DEC DE",            0, REG_IND_DE        , REG_IND_NONE  , DEC},
    {"INC E",             0, REG_IND_E         , REG_IND_NONE  , INC},
    {"DEC E",             0, REG_IND_E         , REG_IND_NONE  , DEC},
    {"LD E,8b",           1, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"RRA",               0, REG_IND_A         , REG_IND_NONE  , RR},
    {"JR NZ,8b",          1, REG_IND_NONE      , REG_IND_NONE  , JRNZ}, //0x20
    {"LD HL,16b",         2, REG_IND_HL        , REG_IND_NONE  , LD16},
    {"LD HL+,A",          0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"INC HL",            0, REG_IND_HL        , REG_IND_NONE  , INC},
    {"INC H",             0, REG_IND_H         , REG_IND_NONE  , INC},
    {"DEC H",             0, REG_IND_H         , REG_IND_NONE  , DEC},
    {"LD H,8b",           1, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"DAA",               0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"JR Z,8b",           1, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"ADD HL,HL",         0, REG_IND_HL        , REG_IND_HL    , ADD16},
    {"LD A,HL+",          0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"DEC HL",            0, REG_IND_HL        , REG_IND_NONE  , DEC}, //indirected but register-local
    {"INC L",             0, REG_IND_L         , REG_IND_NONE  , INC},
    {"DEC L",             0, REG_IND_L         , REG_IND_NONE  , DEC},
    {"LD L,8b",           1, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"CPL",               0, REG_IND_L         , REG_IND_NONE  , CP},
    {"JR NC,8b",          1, REG_IND_NONE      , REG_IND_NONE  , NULL}, //0x30
    {"LD SP,16b",         2, REG_IND_SP        , REG_IND_NONE  , LD16},
    {"LD HL-,A",          0, REG_IND_A         , REG_IND_NONE  , LD_HL_DEC},
    {"INC SP",            0, REG_IND_SP        , REG_IND_NONE  , INC},
    {"INC HL",            0, REG_IND_HL        , REG_IND_NONE  , INC},
    {"DEC(HL)",           0, REG_IND_MEM_HL    , REG_IND_NONE  , NULL}, //indirected and RAM
    {"LD HL,8b",          1, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"SCF",               0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"JR C,8b",           1, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"ADD HL,SP",         0, REG_IND_HL        , REG_IND_SP    , ADD16},
    {"LD A,HL-",          0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"DEC SP",            0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"INC A",             0, REG_IND_A         , REG_IND_NONE  , INC},
    {"DEC A",             0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"LD A,8b",           1, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"CCF",               0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"LD B,B",            0, REG_IND_B         , REG_IND_B     , LDMV}, //0x40
    {"LD B,C",            0, REG_IND_B         , REG_IND_C     , LDMV},
    {"LD B,D",            0, REG_IND_B         , REG_IND_D     , LDMV},
    {"LD B,E",            0, REG_IND_B         , REG_IND_E     , LDMV},
    {"LD B,H",            0, REG_IND_B         , REG_IND_H     , LDMV},
    {"LD B,L",            0, REG_IND_B         , REG_IND_L     , LDMV},
    {"LD B,HL",           0, REG_IND_B         , REG_IND_MEM_HL, LDMV},
    {"LD B,A",            0, REG_IND_B         , REG_IND_A     , LDMV},
    {"LD C,B",            0, REG_IND_C         , REG_IND_B     , LDMV},
    {"LD C,C",            0, REG_IND_C         , REG_IND_C     , LDMV},
    {"LD C,D",            0, REG_IND_C         , REG_IND_D     , LDMV},
    {"LD C,E",            0, REG_IND_C         , REG_IND_E     , LDMV},
    {"LD C,H",            0, REG_IND_C         , REG_IND_H     , LDMV},
    {"LD C,L",            0, REG_IND_C         , REG_IND_L     , LDMV},
    {"LD C,HL",           0, REG_IND_C         , REG_IND_MEM_HL, LDMV},
    {"LD C,A",            0, REG_IND_C         , REG_IND_A     , LDMV},
    {"LD D,B",            0, REG_IND_D         , REG_IND_B     , LDMV}, //0x50
    {"LD D,C",            0, REG_IND_D         , REG_IND_C     , LDMV},
    {"LD D,D",            0, REG_IND_D         , REG_IND_D     , LDMV},
    {"LD D,E",            0, REG_IND_D         , REG_IND_E     , LDMV},
    {"LD D,H",            0, REG_IND_D         , REG_IND_H     , LDMV},
    {"LD D,L",            0, REG_IND_D         , REG_IND_L     , LDMV},
    {"LD D,HL",           0, REG_IND_D         , REG_IND_MEM_HL, LDMV},
    {"LD D,A",            0, REG_IND_D         , REG_IND_A     , LDMV},
    {"LD E,B",            0, REG_IND_E         , REG_IND_B     , LDMV},
    {"LD E,C",            0, REG_IND_E         , REG_IND_C     , LDMV},
    {"LD E,D",            0, REG_IND_E         , REG_IND_D     , LDMV},
    {"LD E,E",            0, REG_IND_E         , REG_IND_E     , LDMV},
    {"LD E,H",            0, REG_IND_E         , REG_IND_H     , LDMV},
    {"LD E,L",            0, REG_IND_E         , REG_IND_L     , LDMV},
    {"LD E,HL",           0, REG_IND_E         , REG_IND_MEM_HL, LDMV},
    {"LD E,A",            0, REG_IND_E         , REG_IND_A     , LDMV},
    {"LD H,B",            0, REG_IND_H         , REG_IND_B     , LDMV}, //0x60
    {"LD H,C",            0, REG_IND_H         , REG_IND_C     , LDMV},
    {"LD H,D",            0, REG_IND_H         , REG_IND_D     , LDMV},
    {"LD H,E",            0, REG_IND_H         , REG_IND_E     , LDMV},
    {"LD H,H",            0, REG_IND_H         , REG_IND_H     , LDMV},
    {"LD H,L",            0, REG_IND_H         , REG_IND_L     , LDMV},
    {"LD H,HL",           0, REG_IND_H         , REG_IND_MEM_HL, LDMV},
    {"LD H,A",            0, REG_IND_H         , REG_IND_A     , LDMV},
    {"LD L,B",            0, REG_IND_L         , REG_IND_B     , LDMV},
    {"LD L,C",            0, REG_IND_L         , REG_IND_C     , LDMV},
    {"LD L,D",            0, REG_IND_L         , REG_IND_D     , LDMV},
    {"LD L,E",            0, REG_IND_L         , REG_IND_E     , LDMV},
    {"LD L,H",            0, REG_IND_L         , REG_IND_H     , LDMV},
    {"LD L,L",            0, REG_IND_L         , REG_IND_L     , LDMV},
    {"LD L,HL",           0, REG_IND_L         , REG_IND_MEM_HL, LDMV},
    {"LD L,A",            0, REG_IND_L         , REG_IND_A     , LDMV},
    {"LD HL,B",           0, REG_IND_MEM_HL    , REG_IND_B     , LDMV}, //0x70
    {"LD HL,C",           0, REG_IND_MEM_HL    , REG_IND_C     , LDMV},
    {"LD HL,D",           0, REG_IND_MEM_HL    , REG_IND_D     , LDMV},
    {"LD HL,E",           0, REG_IND_MEM_HL    , REG_IND_E     , LDMV},
    {"LD HL,H",           0, REG_IND_MEM_HL    , REG_IND_H     , LDMV},
    {"LD HL,L",           0, REG_IND_MEM_HL    , REG_IND_L     , LDMV},
    {"HALT",              0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"LD HL,A",           0, REG_IND_MEM_HL    , REG_IND_A     , LDMV},
    {"LD A,B",            0, REG_IND_A         , REG_IND_B     , LDMV},
    {"LD A,C",            0, REG_IND_A         , REG_IND_C     , LDMV},
    {"LD A,D",            0, REG_IND_A         , REG_IND_D     , LDMV},
    {"LD A,E",            0, REG_IND_A         , REG_IND_E     , LDMV},
    {"LD A,H",            0, REG_IND_A         , REG_IND_H     , LDMV},
    {"LD A,L",            0, REG_IND_A         , REG_IND_L     , LDMV},
    {"LD A,HL",           0, REG_IND_A         , REG_IND_HL    , LDMV},
    {"LD A,A",            0, REG_IND_A         , REG_IND_A     , LDMV},
    {"ADD A,B",           0, REG_IND_NONE      , REG_IND_NONE  , NULL}, //0x80
    {"ADD A,C",           0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"ADD A,D",           0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"ADD A,E",           0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"ADD A,H",           0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"ADD A,L",           0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"ADD A,HL",          0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"ADD A,A",           0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"ADC A,B",           0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"ADC A,C",           0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"ADC A,D",           0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"ADC A,E",           0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"ADC A,H",           0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"ADC A,L",           0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"ADC A,HL",          0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"ADC A,A",           0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"SUB B",             0, REG_IND_NONE      , REG_IND_NONE  , NULL}, //0x90
    {"SUB C",             0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"SUB D",             0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"SUB E",             0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"SUB H",             0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"SUB L",             0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"SUB HL",            0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"SUB A",             0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"SBC A,B",           0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"SBC A,C",           0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"SBC A,D",           0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"SBC A,E",           0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"SBC A,H",           0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"SBC A,L",           0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"SBC A,HL",          0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"SBC A,A",           0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"AND B",             0, REG_IND_NONE      , REG_IND_NONE  , NULL}, //0xA0
    {"AND C",             0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"AND D",             0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"AND E",             0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"AND H",             0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"AND L",             0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"AND HL",            0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"AND A",             0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"XOR B",             0, REG_IND_B         , REG_IND_NONE  , XOR},
    {"XOR C",             0, REG_IND_C         , REG_IND_NONE  , XOR},
    {"XOR D",             0, REG_IND_D         , REG_IND_NONE  , XOR},
    {"XOR E",             0, REG_IND_E         , REG_IND_NONE  , XOR},
    {"XOR H",             0, REG_IND_H         , REG_IND_NONE  , XOR},
    {"XOR L",             0, REG_IND_L         , REG_IND_NONE  , XOR},
    {"XOR HL",            0, REG_IND_HL        , REG_IND_NONE  , XOR},
    {"XOR A",             0, REG_IND_A         , REG_IND_NONE  , XOR},
    {"OR B",              0, REG_IND_B         , REG_IND_NONE  , OR}, //0xB0
    {"OR C",              0, REG_IND_C         , REG_IND_NONE  , OR},
    {"OR D",              0, REG_IND_D         , REG_IND_NONE  , OR},
    {"OR E",              0, REG_IND_E         , REG_IND_NONE  , OR},
    {"OR H",              0, REG_IND_H         , REG_IND_NONE  , OR},
    {"OR L",              0, REG_IND_L         , REG_IND_NONE  , OR},
    {"OR HL",             0, REG_IND_MEM_HL    , REG_IND_NONE  , OR},
    {"OR A",              0, REG_IND_A         , REG_IND_NONE  , OR},
    {"CP B",              0, REG_IND_B         , REG_IND_NONE  , CP},
    {"CP C",              0, REG_IND_C         , REG_IND_NONE  , CP},
    {"CP D",              0, REG_IND_D         , REG_IND_NONE  , CP},
    {"CP E",              0, REG_IND_E         , REG_IND_NONE  , CP},
    {"CP H",              0, REG_IND_H         , REG_IND_NONE  , CP},
    {"CP L",              0, REG_IND_L         , REG_IND_NONE  , CP},
    {"CP HL",             0, REG_IND_MEM_HL    , REG_IND_NONE  , CP},
    {"CP A",              0, REG_IND_A         , REG_IND_NONE  , CP},
    {"RET NZ",            0, REG_IND_NONE      , REG_IND_NONE  , NULL}, //0xC0
    {"POP BC",            0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"JP NZ,16b",         2, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"JP 16b",            2, REG_IND_NONE      , REG_IND_NONE  , jump},
    {"CALL NZ,16b",       2, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"PUSH BC",           0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"ADD A,8b",          1, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"RST 00",            0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"RET Z",             0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"RET",               0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"JP Z,16b",          0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"PREFIX CB",         0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"CALL Z,16b",        0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"CALL 16b",          0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"ADC A,8b",          0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"RST 08",            0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"RET NC",            0, REG_IND_NONE      , REG_IND_NONE  , NULL}, //0xD0
    {"POP DE",            0, REG_IND_NONE      , REG_IND_NONE  , NULL},
    {"JP NZ,16b",         2, REG_IND_NONE      , REG_IND_NONE  , JPNZ},
};

cpu::cpu(uint8_t* p_rom_input){
    p_rom = p_rom_input;
    reg.PC = 0;
}

uint16_t find_16_from_8(uint8_t* array, int ptr){
    uint8_t low = array[ptr];
    uint8_t high = array[ptr+1];
    uint16_t combined = (uint16_t)high;
    combined = combined << 8;
    combined = combined + low;
    return combined;
}



int cpu::execute_opcode(){
    uint16_t operand = 0;
    // printf("current program counter: 0x%04X\n", reg.PC);
    uint8_t curr_opcode = p_rom[reg.PC];
    printf("current opcode = 0x%02X\n", curr_opcode);
    opcode ctx = opcodes[curr_opcode];
    printf("which is instruction %s", ctx.str_name);

    if(ctx.operand_length == 1){
        operand = p_rom[reg.PC+1];
        printf("(0x%02X)\n", operand);

    }
    else if(ctx.operand_length == 2){
        operand = find_16_from_8(p_rom, reg.PC+1);
        printf("(0x%04X)\n", operand);

    }
    else{
        printf("\n");
    }

    if(ctx.opcode_pointer == NULL){
        printf("instruction not implemented :(\n");
    } else{
        reg.PC_next = reg.PC + ctx.operand_length + 1;
        ctx.opcode_pointer(operand, &reg, ram, ctx.reg_ind, ctx.reg_ind_two);
        reg.PC_step();
        return 0;
    }
    return -1;
}

void cpu::dump_registers(){
    printf("Register dump\n");
    printf("A: 0x%02X F: 0x%02X\n", reg.A, reg.F.byte);
    printf("B: 0x%02X C: 0x%02X\n", reg.B, reg.C);
    printf("D: 0x%02X E: 0x%02X\n", reg.D, reg.E);
    printf("H: 0x%02X L: 0x%02X\n", reg.H, reg.L);
    printf("PC: 0x%04X\n", reg.PC);
}