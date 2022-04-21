#include <cpu.hpp>
#include <registers.hpp>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <byteswap.h>

void jump(uint16_t address, registers_t* p_reg, reg_ind_t ind){
    p_reg->PC = address;
}

void XOR(uint16_t operand, registers_t* p_reg, reg_ind_t ind){
    switch(ind){
        case REG_IND_A:
            p_reg->A = p_reg->A ^ p_reg->A;
            break;
        case REG_IND_B:
            p_reg->A = p_reg->A ^ p_reg->B;
            break;
        case REG_IND_C:
            p_reg->A = p_reg->A ^ p_reg->C;
            break;
        case REG_IND_D:
            p_reg->A = p_reg->A ^ p_reg->D;
            break;
        case REG_IND_E:
            p_reg->A = p_reg->A ^ p_reg->E;
            break;
        case REG_IND_H:
            p_reg->A = p_reg->A ^ p_reg->H;
            break;
        case REG_IND_L:
            p_reg->A = p_reg->A ^ p_reg->L;
            break;
        case REG_IND_HL:
            p_reg->A = p_reg->A ^ p_reg->HL;
            break;
        default:
            p_reg->A = p_reg->A ^ (operand & 0xFF);
    }
    p_reg->F.bits.z = (p_reg->A == 0) ? true : false;
}

void LDHL16(uint16_t operand, registers_t* p_reg, reg_ind_t ind){
    p_reg->HL = operand;
}

void LD8(uint16_t operand, registers_t* p_reg, reg_ind_t ind){
    switch(ind){
        case REG_IND_A:
            p_reg->A = (operand & 0xFF);
            break;
        case REG_IND_B:
            p_reg->B = (operand & 0xFF);
            break;
        case REG_IND_C:
            p_reg->C = (operand & 0xFF);
            break;
        case REG_IND_D:
            p_reg->C = (operand & 0xFF);
            break;
        case REG_IND_E:
            p_reg->E = (operand & 0xFF);
            break;
        case REG_IND_H:
            p_reg->H = (operand & 0xFF);
            break;
        case REG_IND_L:
            p_reg->L = (operand & 0xFF);
            break;
        case REG_IND_HL:
            p_reg->HL = (operand & 0xFF);
            break;
        default:
            exit(-1);
    }
}

opcode opcodes[256] = {
    {"NOP",               0, REG_IND_NONE, NULL}, //0x00
    {"LD BC,16b",         2, REG_IND_NONE, NULL},
    {"LD (BC), A",        0, REG_IND_NONE, NULL},
    {"INC BC",            0, REG_IND_NONE, NULL},
    {"INC B",             0, REG_IND_NONE, NULL},
    {"DEC B",             0, REG_IND_NONE, NULL},
    {"LD B,8b",           1, REG_IND_B   , LD8 },
    {"RLCA",              0, REG_IND_NONE, NULL},
    {"LD 16b,SP",         2, REG_IND_NONE, NULL},
    {"ADD HL,BC",         0, REG_IND_NONE, NULL},
    {"LD A,(BC)",         0, REG_IND_NONE, NULL},
    {"DEC BC",            0, REG_IND_NONE, NULL},
    {"INC C",             0, REG_IND_NONE, NULL},
    {"DEC C",             0, REG_IND_NONE, NULL},
    {"LD C,8b",           1, REG_IND_C   , LD8},
    {"RRCA",              0, REG_IND_NONE, NULL}, //0x10
    {"STOP 0",            1, REG_IND_NONE, NULL},
    {"LD DE,16b",         2, REG_IND_NONE, NULL},
    {"LD DE, A",          0, REG_IND_NONE, NULL},
    {"INC DE",            0, REG_IND_NONE, NULL},
    {"INC D",             0, REG_IND_NONE, NULL},
    {"DEC D",             0, REG_IND_NONE, NULL},
    {"LD D 8b",           1, REG_IND_NONE, NULL},
    {"RLA",               0, REG_IND_NONE, NULL},
    {"JR 8b",             1, REG_IND_NONE, NULL},
    {"ADD HL,DE",         0, REG_IND_NONE, NULL},
    {"LD A, DE",          0, REG_IND_NONE, NULL},
    {"DEC DE",            0, REG_IND_NONE, NULL},
    {"INC E",             0, REG_IND_NONE, NULL},
    {"DEC E",             0, REG_IND_NONE, NULL},
    {"LD E,8b",           1, REG_IND_NONE, NULL},
    {"RRA",               0, REG_IND_NONE, NULL},
    {"JR NZ,8b",          1, REG_IND_NONE, NULL}, //0x20
    {"LD HL,16b",         2, REG_IND_NONE, LDHL16},
    {"LD HL+,A",          0, REG_IND_NONE, NULL},
    {"INC HL",            0, REG_IND_NONE, NULL},
    {"INC H",             0, REG_IND_NONE, NULL},
    {"DEC H",             0, REG_IND_NONE, NULL},
    {"LD H,8b",           1, REG_IND_NONE, NULL},
    {"DAA",               0, REG_IND_NONE, NULL},
    {"JR Z,8b",           1, REG_IND_NONE, NULL},
    {"ADD HL,HL",         0, REG_IND_NONE, NULL},
    {"LD A,HL+",          0, REG_IND_NONE, NULL},
    {"DEC HL",            0, REG_IND_NONE, NULL},
    {"INC L",             0, REG_IND_NONE, NULL},
    {"DEC L",             0, REG_IND_NONE, NULL},
    {"LD L,8b",           1, REG_IND_NONE, NULL},
    {"CPL",               0, REG_IND_NONE, NULL},
    {"JR NC,8b",          1, REG_IND_NONE, NULL}, //0x30
    {"LD SP,16b",         2, REG_IND_NONE, NULL},
    {"LD HL-,A",          0, REG_IND_NONE, NULL},
    {"INC SP",            0, REG_IND_NONE, NULL},
    {"INC HL",            0, REG_IND_NONE, NULL},
    {"DEC HL",            0, REG_IND_NONE, NULL},
    {"LD HL,8b",          1, REG_IND_NONE, NULL},
    {"SCF",               0, REG_IND_NONE, NULL},
    {"JR C,8b",           1, REG_IND_NONE, NULL},
    {"ADD HL,SP",         0, REG_IND_NONE, NULL},
    {"LD A,HL-",          0, REG_IND_NONE, NULL},
    {"DEC SP",            0, REG_IND_NONE, NULL},
    {"INC A",             0, REG_IND_NONE, NULL},
    {"DEC A",             0, REG_IND_NONE, NULL},
    {"LD A,8b",           1, REG_IND_NONE, NULL},
    {"CCF",               0, REG_IND_NONE, NULL},
    {"LD B,B",            0, REG_IND_NONE, NULL}, //0x40
    {"LD B,C",            0, REG_IND_NONE, NULL},
    {"LD B,D",            0, REG_IND_NONE, NULL},
    {"LD B,E",            0, REG_IND_NONE, NULL},
    {"LD B,H",            0, REG_IND_NONE, NULL},
    {"LD B,L",            0, REG_IND_NONE, NULL},
    {"LD B,HL",           0, REG_IND_NONE, NULL},
    {"LD B,A",            0, REG_IND_NONE, NULL},
    {"LD C,B",            0, REG_IND_NONE, NULL},
    {"LD C,C",            0, REG_IND_NONE, NULL},
    {"LD C,D",            0, REG_IND_NONE, NULL},
    {"LD C,E",            0, REG_IND_NONE, NULL},
    {"LD C,H",            0, REG_IND_NONE, NULL},
    {"LD C,L",            0, REG_IND_NONE, NULL},
    {"LD C,HL",           0, REG_IND_NONE, NULL},
    {"LD C,A",            0, REG_IND_NONE, NULL},
    {"LD D,B",            0, REG_IND_NONE, NULL}, //0x50
    {"LD D,C",            0, REG_IND_NONE, NULL},
    {"LD D,D",            0, REG_IND_NONE, NULL},
    {"LD D,E",            0, REG_IND_NONE, NULL},
    {"LD D,H",            0, REG_IND_NONE, NULL},
    {"LD D,L",            0, REG_IND_NONE, NULL},
    {"LD D,HL",           0, REG_IND_NONE, NULL},
    {"LD D,A",            0, REG_IND_NONE, NULL},
    {"LD E,B",            0, REG_IND_NONE, NULL},
    {"LD E,C",            0, REG_IND_NONE, NULL},
    {"LD E,D",            0, REG_IND_NONE, NULL},
    {"LD E,E",            0, REG_IND_NONE, NULL},
    {"LD E,H",            0, REG_IND_NONE, NULL},
    {"LD E,L",            0, REG_IND_NONE, NULL},
    {"LD E,HL",           0, REG_IND_NONE, NULL},
    {"LD E,A",            0, REG_IND_NONE, NULL},
    {"LD H,B",            0, REG_IND_NONE, NULL}, //0x60
    {"LD H,C",            0, REG_IND_NONE, NULL},
    {"LD H,D",            0, REG_IND_NONE, NULL},
    {"LD H,E",            0, REG_IND_NONE, NULL},
    {"LD H,H",            0, REG_IND_NONE, NULL},
    {"LD H,L",            0, REG_IND_NONE, NULL},
    {"LD H,HL",           0, REG_IND_NONE, NULL},
    {"LD H,A",            0, REG_IND_NONE, NULL},
    {"LD L,B",            0, REG_IND_NONE, NULL},
    {"LD L,C",            0, REG_IND_NONE, NULL},
    {"LD L,D",            0, REG_IND_NONE, NULL},
    {"LD L,E",            0, REG_IND_NONE, NULL},
    {"LD L,H",            0, REG_IND_NONE, NULL},
    {"LD L,L",            0, REG_IND_NONE, NULL},
    {"LD L,HL",           0, REG_IND_NONE, NULL},
    {"LD L,A",            0, REG_IND_NONE, NULL},
    {"LD HL,B",           0, REG_IND_NONE, NULL}, //0x70
    {"LD HL,C",           0, REG_IND_NONE, NULL},
    {"LD HL,D",           0, REG_IND_NONE, NULL},
    {"LD HL,E",           0, REG_IND_NONE, NULL},
    {"LD HL,H",           0, REG_IND_NONE, NULL},
    {"LD HL,L",           0, REG_IND_NONE, NULL},
    {"HALT",              0, REG_IND_NONE, NULL},
    {"LD HL,A",           0, REG_IND_NONE, NULL},
    {"LD A,B",            0, REG_IND_NONE, NULL},
    {"LD A,C",            0, REG_IND_NONE, NULL},
    {"LD A,D",            0, REG_IND_NONE, NULL},
    {"LD A,E",            0, REG_IND_NONE, NULL},
    {"LD A,H",            0, REG_IND_NONE, NULL},
    {"LD A,L",            0, REG_IND_NONE, NULL},
    {"LD A,HL",           0, REG_IND_NONE, NULL},
    {"LD A,A",            0, REG_IND_NONE, NULL},
    {"ADD A,B",           0, REG_IND_NONE, NULL}, //0x80
    {"ADD A,C",           0, REG_IND_NONE, NULL},
    {"ADD A,D",           0, REG_IND_NONE, NULL},
    {"ADD A,E",           0, REG_IND_NONE, NULL},
    {"ADD A,H",           0, REG_IND_NONE, NULL},
    {"ADD A,L",           0, REG_IND_NONE, NULL},
    {"ADD A,HL",          0, REG_IND_NONE, NULL},
    {"ADD A,A",           0, REG_IND_NONE, NULL},
    {"ADC A,B",           0, REG_IND_NONE, NULL},
    {"ADC A,C",           0, REG_IND_NONE, NULL},
    {"ADC A,D",           0, REG_IND_NONE, NULL},
    {"ADC A,E",           0, REG_IND_NONE, NULL},
    {"ADC A,H",           0, REG_IND_NONE, NULL},
    {"ADC A,L",           0, REG_IND_NONE, NULL},
    {"ADC A,HL",          0, REG_IND_NONE, NULL},
    {"ADC A,A",           0, REG_IND_NONE, NULL},
    {"SUB B",             0, REG_IND_NONE, NULL}, //0x90
    {"SUB C",             0, REG_IND_NONE, NULL},
    {"SUB D",             0, REG_IND_NONE, NULL},
    {"SUB E",             0, REG_IND_NONE, NULL},
    {"SUB H",             0, REG_IND_NONE, NULL},
    {"SUB L",             0, REG_IND_NONE, NULL},
    {"SUB HL",            0, REG_IND_NONE, NULL},
    {"SUB A",             0, REG_IND_NONE, NULL},
    {"SBC A,B",           0, REG_IND_NONE, NULL},
    {"SBC A,C",           0, REG_IND_NONE, NULL},
    {"SBC A,D",           0, REG_IND_NONE, NULL},
    {"SBC A,E",           0, REG_IND_NONE, NULL},
    {"SBC A,H",           0, REG_IND_NONE, NULL},
    {"SBC A,L",           0, REG_IND_NONE, NULL},
    {"SBC A,HL",          0, REG_IND_NONE, NULL},
    {"SBC A,A",           0, REG_IND_NONE, NULL},
    {"AND B",             0, REG_IND_NONE, NULL}, //0xA0
    {"AND C",             0, REG_IND_NONE, NULL},
    {"AND D",             0, REG_IND_NONE, NULL},
    {"AND E",             0, REG_IND_NONE, NULL},
    {"AND H",             0, REG_IND_NONE, NULL},
    {"AND L",             0, REG_IND_NONE, NULL},
    {"AND HL",            0, REG_IND_NONE, NULL},
    {"AND A",             0, REG_IND_NONE, NULL},
    {"XOR B",             0, REG_IND_B   , XOR},
    {"XOR C",             0, REG_IND_C   , XOR},
    {"XOR D",             0, REG_IND_D   , XOR},
    {"XOR E",             0, REG_IND_E   , XOR},
    {"XOR H",             0, REG_IND_H   , XOR},
    {"XOR L",             0, REG_IND_L   , XOR},
    {"XOR HL",            0, REG_IND_HL  , XOR},
    {"XOR A",             0, REG_IND_A   , XOR},
    {"OR B",              0, REG_IND_NONE, NULL}, //0xB0
    {"OR C",              0, REG_IND_NONE, NULL},
    {"OR D",              0, REG_IND_NONE, NULL},
    {"OR E",              0, REG_IND_NONE, NULL},
    {"OR H",              0, REG_IND_NONE, NULL},
    {"OR L",              0, REG_IND_NONE, NULL},
    {"OR HL",             0, REG_IND_NONE, NULL},
    {"OR A",              0, REG_IND_NONE, NULL},
    {"CP B",              0, REG_IND_NONE, NULL},
    {"CP C",              0, REG_IND_NONE, NULL},
    {"CP D",              0, REG_IND_NONE, NULL},
    {"CP E",              0, REG_IND_NONE, NULL},
    {"CP H",              0, REG_IND_NONE, NULL},
    {"CP L",              0, REG_IND_NONE, NULL},
    {"CP HL",             0, REG_IND_NONE, NULL},
    {"CP A",              0, REG_IND_NONE, NULL},
    {"RET NZ",            0, REG_IND_NONE, NULL},
    {"POP BC",            0, REG_IND_NONE, NULL},
    {"JP NZ,16b",         2, REG_IND_NONE, NULL},
    {"JP 16b",            2, REG_IND_NONE, jump},
    {"CALL NZ,16b",       2, REG_IND_NONE, NULL},
    {"PUSH BC",           0, REG_IND_NONE, NULL},
    {"ADD A,8b",          1, REG_IND_NONE, NULL},
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
        reg.PC += ctx.operand_length + 1;
        ctx.opcode_pointer(operand, &reg, ctx.reg_ind);
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