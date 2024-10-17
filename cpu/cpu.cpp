#include <registers.hpp>
#include <interrupts.hpp>
#include <ram.hpp>
#include <cpu.hpp>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <byteswap.h>
#include <renderer.hpp>

extern register_t reg;
extern renderer_t ren;
cpu_t cpu;

static void JUMP(uint16_t address, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(ind == IND_MEM_HL){
        reg_write(IND_PC_NEXT, reg_read(IND_HL));
        cpu.ticks += 4;
    } else{
        reg_write(IND_PC_NEXT, address);
        cpu.ticks += 12;
    }
}

static void JR(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    reg_write(IND_PC_NEXT, reg_read(IND_PC) + (operand & 0xFF));
    cpu.ticks += 8;
}

static void LD_HL_DEC(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    ram_write8(reg_read(IND_HL), reg_read(ind));
    reg_write(IND_HL, reg_read(IND_HL) - 1);
    cpu.ticks += 8;
}

static void LD_HL_INC(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    reg_write(IND_A, ram_read8(reg_read(IND_HL)));
    reg_write(IND_HL, reg_read(IND_HL) + 1);
    cpu.ticks += 8;
}

static void LDA_HL_INC(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    ram_write8(reg_read(IND_HL), reg_read(IND_A));
    reg_write(IND_HL, reg_read(IND_HL) + 1);
    cpu.ticks += 8;
}

static void XOR(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(ind == IND_HL){
        reg_write(IND_A, (ram_read8(reg_read(IND_HL)) ^ (operand & 0xFF)));
        reg.F.bits.z = (reg_read(IND_A) == 0) ? true : false;
        cpu.ticks += 8;
    } else if(ind == IND_NONE){
        reg_write(IND_A, (ram_read8(operand) ^ (operand & 0xFF)));
        reg.F.bits.z = (reg_read(IND_A) == 0) ? true : false;
        cpu.ticks += 8;
    }
    reg_write(IND_A, (reg_read(ind) ^ (operand & 0xFF)));
    reg.F.bits.z = (reg_read(IND_A) == 0) ? true : false;
    cpu.ticks += 4;
}

static void DEC(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(ind == IND_MEM_HL){
        ram_dec(reg_read(IND_HL));
        reg.F.bits.z = (ram_read8(reg_read(IND_HL)) == 0) ? true : false;
        reg.F.bits.h = ((ram_read8(reg_read(IND_HL)) & 0x0F) == 0x0F);
        cpu.ticks += 12;
    } else if(ind == IND_BC || ind == IND_DE || ind == IND_HL || ind == IND_SP){
        reg_write(ind, reg_read(ind) - 1);
        reg.F.bits.z = (reg_read(ind) == 0) ? true : false;
        reg.F.bits.h = ((reg_read(ind) & 0x0FFF) == 0x0000);
        cpu.ticks += 8;
    } else {
        reg_write(ind, reg_read(ind) - 1);
        reg.F.bits.z = (reg_read(ind) == 0) ? true : false;
        reg.F.bits.h = ((reg_read(ind) & 0x0F) == 0x0F);
        cpu.ticks += 4;
    }
}

static void INC(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(ind == IND_MEM_HL){ //8 bit indirected
        ram_inc(reg_read(IND_HL));
        reg.F.bits.z = (ram_read8(reg_read(IND_HL)) == 0) ? true : false;
        reg.F.bits.h = ((ram_read8(reg_read(IND_HL)) & 0x0F) == 0x00);
        cpu.ticks += 12;
    } else if(ind == IND_BC || ind == IND_DE || ind == IND_HL || ind == IND_SP){ //16 bit direct
        reg_write(ind, reg_read(ind) + 1);
        reg.F.bits.z = (reg_read(ind) == 0) ? true : false;
        reg.F.bits.h = ((reg_read(ind) & 0x0FFF) == 0x0000);
        cpu.ticks += 8;
    } else { //8 bit direct
        reg_write(ind, reg_read(ind) + 1);
        reg.F.bits.z = (reg_read(ind) == 0) ? true : false;
        reg.F.bits.h = ((reg_read(ind) & 0x0F) == 0x00);
        cpu.ticks += 4;
    }
}

static void LD16(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(ind == IND_NONE){
        ram_write8(operand, reg_read(ind2));
        cpu.ticks += 16;
    }
    else{
        reg_write(ind, operand);
        cpu.ticks += 8;
    }
}

static void LDA_8_IND(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(ind == IND_BC || ind == IND_DE || ind == IND_HL || ind == IND_SP){
        reg_write(IND_A, ram_read8(reg_read(ind)));
        cpu.ticks += 8;
    } else if(ind == IND_NONE){
        reg_write(IND_A, ram_read8(operand));
        cpu.ticks += 16;
    } else{
        reg_write(IND_A, reg_read(ind));
        cpu.ticks += 4;
    }
}

static void LD8(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    reg_write(ind, (operand & 0xFF));
    cpu.ticks += 8;
}

static void JRNZ(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(!reg.F.bits.z){
        reg.PC_next = reg.PC + (int8_t)((operand & 0xFF) + 2); //cast to signed int for hopefully subtraction
    }
    cpu.ticks += 8;
}

static void JRZ(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(reg.F.bits.z){
        reg.PC_next = reg.PC + (int8_t)((operand & 0xFF) + 2); //cast to signed int for hopefully subtraction
    }
    cpu.ticks += 8;
}

static void JPNC(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(!reg.F.bits.cy){
        reg.PC_next = reg.PC + (operand & 0xFF);
    }
    cpu.ticks += 8;
}

static void JPNZ(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(!reg.F.bits.z){
        reg.PC_next = reg.PC + (operand & 0xFF);
    }
    cpu.ticks += 8;
}

//rotate through the carry flag 
static void RR(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(ind == IND_MEM_HL){
        uint8_t carry_old = reg.F.bits.cy;
        reg.F.bits.cy = (ram_read8((reg_read(IND_HL))) & 0x01);
        ram_write8(reg_read(IND_HL), (ram_read8((reg_read(IND_HL)))  >> 1) ^ carry_old << 8);
        cpu.ticks += 16;
    }
    else{
        uint8_t carry_old = reg.F.bits.cy;
        reg.F.bits.cy = (reg_read(ind) & 0x01);
        reg_write(ind, (reg_read(ind) >> 1) ^ carry_old << 8);
        cpu.ticks += 8;
    }
}

//rotate right, move old bit 0 to carry flag as well
static void RRC(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(ind == IND_MEM_HL){
        reg.F.bits.cy = (ram_read8(reg_read(IND_HL)) & 0x01);
        ram_write8(reg_read(IND_HL), (ram_read8(reg_read(IND_HL)) >> 1) ^ reg.F.bits.cy  << 8);
        cpu.ticks += 16;
    }
    reg.F.bits.cy = (reg_read(ind) & 0x80);
    reg_write(ind, (reg_read(ind) >> 1) ^ reg.F.bits.cy << 8);
    cpu.ticks += 8;
}

static void SRA(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(ind == IND_MEM_HL){
        uint8_t old_MSB = ram_read8(reg_read(IND_HL)) & 0x80;
        uint8_t old_LSB = ram_read8(reg_read(IND_HL)) & 0x01;
        reg.F.bits.cy = old_LSB;
        ram_write8(reg_read(IND_HL), (ram_read8(reg_read(IND_HL)) >> 1) ^ old_MSB << 8);
    }
    else{
        uint8_t old_MSB = ram_read8(reg_read(ind)) & 0x80;
        uint8_t old_LSB = ram_read8(reg_read(ind)) & 0x01;
        reg.F.bits.cy = old_LSB;
        reg_write(ind, (reg_read(ind) >> 1) ^ old_MSB << 8);
    }
}

static void SRL(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(ind == IND_MEM_HL){
        uint8_t old_LSB = ram_read8(reg_read(ind)) & 0x01;
        reg.F.bits.cy = old_LSB;
        ram_write8(reg_read(IND_HL), (ram_read8(reg_read(IND_HL)) >> 1) ^ 0 << 8);
    }
    else{
        uint8_t old_LSB = ram_read8(reg_read(ind)) & 0x01;
        reg.F.bits.cy = old_LSB;
        reg_write(ind, (reg_read(ind) >> 1) ^ 0 << 8);
    }
}

static void RL(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(ind == IND_MEM_HL){
        uint8_t carry_old = reg.F.bits.cy;
        reg.F.bits.cy = (ram_read8((reg_read(IND_HL))) & 0x80);
        reg_write(ind, (ram_read8((reg_read(IND_HL)))  << 1) ^ carry_old >> 8);
        cpu.ticks += 16;
    }
    else{
        uint8_t carry_old = reg.F.bits.cy;
        reg.F.bits.cy = (reg_read(ind) & 0x80);
        reg_write(ind, (reg_read(ind) << 1) ^ carry_old >> 8);
        cpu.ticks += 8;
    }
}

static void RLC(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(ind == IND_MEM_HL){
        reg.F.bits.cy = (ram_read8(reg_read(IND_HL)) & 0x80);
        reg_write(ind, (ram_read8(reg_read(IND_HL)) << 1));
        cpu.ticks += 8;
    }
    reg.F.bits.cy = (reg_read(ind) & 0x80);
    reg_write(ind, (reg_read(ind) << 1));
    cpu.ticks += 4;
}

static void OR(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(ind == IND_MEM_HL){
        reg_write(IND_A, (ram_read8(reg_read(IND_HL)) | reg_read(IND_A)));
        reg.F.bits.z = (reg_read(IND_A) == 0) ? true : false;
        cpu.ticks += 8;
    } else if(ind == IND_NONE){
        reg_write(IND_A, ((operand & 0xFF) | reg_read(IND_A)));
        reg.F.bits.z = (reg_read(IND_A) == 0) ? true : false;
        cpu.ticks += 8;
    } else{
        reg_write(IND_A, (reg_read(ind) | reg_read(IND_A)));
        reg.F.bits.z = (reg_read(IND_A) == 0) ? true : false;
        cpu.ticks += 4;
    }
}

static void AND(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(ind == IND_MEM_HL){
        reg_write(IND_A, (ram_read8(reg_read(IND_HL)) & reg_read(IND_A)));
        reg.F.bits.z = (reg_read(IND_A) == 0) ? true : false;
        cpu.ticks += 8;
    } else if(ind == IND_NONE){
        reg_write(IND_A, ((operand & 0xFF) & reg_read(IND_A)));
        reg.F.bits.z = (reg_read(IND_A) == 0) ? true : false;
        cpu.ticks += 8;
    } else{
    reg_write(IND_A, (reg_read(ind) & reg_read(IND_A)));
    reg.F.bits.z = (reg_read(IND_A) == 0) ? true : false;
    cpu.ticks += 4;
    }
}

static void LDMV(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(ind == IND_MEM_HL){
        ram_write8(reg_read(IND_HL), reg_read(ind2));
        cpu.ticks += 8;
    }
    else if(ind == IND_DE){
        ram_write8(reg_read(IND_DE), reg_read(ind2));
        cpu.ticks += 8;
    }
    else if(ind2 == IND_MEM_HL){
        reg_write(ind, ram_read8(reg_read(IND_HL)));
    }
    else if (ind == IND_NONE){
        reg_write(ind2, ram_read8(operand));
        cpu.ticks += 8;
    }
    else{
        reg_write(ind, (reg_read(ind2)));
        cpu.ticks += 4;
    }
}

static void LDDIR(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(ind == IND_NONE){
        reg_write(ind2, ram_read8(0xFF00 + (operand & 0xFF)));
        cpu.ticks += 12;
    } 
    else if(ind2 == IND_NONE){
        ram_write8(0xFF00 + (operand & 0xFF), reg_read(ind));
        cpu.ticks += 12;
    }
    else{
        ram_write8(0xFF00 + reg_read(ind), reg_read(ind2));
        cpu.ticks += 8;
    }
}

static void LD_DIR_16(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    ram_write8(reg_read(ind), reg_read(ind2));
    cpu.ticks += 8;
}

static void CP(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(ind == IND_MEM_HL){
        uint8_t tmp = reg_read(IND_A) - ram_read8(reg_read(IND_HL));
        reg.F.bits.z = (tmp == 0) ? true : false;
        reg.F.bits.h = (tmp & 0x0F) == 0x0F;
        reg.F.bits.cy = (reg_read(IND_A) < ram_read8(reg_read(IND_HL))) ? true : false;
        cpu.ticks += 8;
    } else if(ind == IND_NONE) {
        uint8_t tmp = reg_read(IND_A) - (operand & 0xFF);
        reg.F.bits.z = (tmp == 0) ? true : false;
        reg.F.bits.h = (tmp & 0x0F) == 0x0F;
        reg.F.bits.cy = (reg_read(IND_A) < reg_read(ind)) ? true : false;
        cpu.ticks += 8;
    } else {
        uint8_t tmp = reg_read(IND_A) - reg_read(ind);
        reg.F.bits.z = (tmp == 0) ? true : false;
        reg.F.bits.h = (tmp & 0x0F) == 0x0F;
        reg.F.bits.cy = (reg_read(IND_A) < reg_read(ind)) ? true : false;
        cpu.ticks += 4;
    }
}

static void ADD16(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    reg_write(ind, reg_read(ind) + reg_read(ind2));
    reg.F.bits.n = false;
    reg.F.bits.h = (((reg_read(ind) & 0xF000) + (reg_read(ind2) & 0xF000)) & 0x1000) == 0x1000;
    reg.F.bits.cy = (reg_read(ind) > UINT16_MAX - reg_read(ind2)) ? true : false;
    cpu.ticks += 8;
}

static void ADC8(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(ind == IND_MEM_HL){
        reg_write(IND_A, (reg_read(IND_A) + ram_read8(reg_read(IND_HL)) + reg.F.bits.cy));
        reg.F.bits.cy = (reg_read(IND_A) > UINT16_MAX - ram_read8(reg_read(IND_HL))) ? true : false;
        cpu.ticks += 8;
    } else{
        reg_write(IND_A, (reg_read(IND_A) + reg_read(ind) + regF.bits.cy));
        reg.F.bits.cy = (reg_read(IND_A) > UINT16_MAX - reg_read(ind)) ? true : false;
        cpu.ticks += 4;
    }
    reg.F.bits.z = (reg_read(IND_A) == 0) ? true : false;
    reg.F.bits.h = ((reg_read(ind) & 0x0F) == 0x00);
    reg.F.bits.cy = (reg_read(IND_A) > UINT16_MAX - reg_read(ind)) ? true : false;
}

static void ADD8(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(ind == IND_MEM_HL){
        reg_write(IND_A, (reg_read(IND_A) + ram_read8(reg_read(IND_HL))));
        reg.F.bits.cy = (reg_read(IND_A) > UINT16_MAX - ram_read8(reg_read(IND_HL))) ? true : false;
        cpu.ticks += 8;
    } else{
        reg_write(IND_A, (reg_read(IND_A) + reg_read(ind)));
        reg.F.bits.cy = (reg_read(IND_A) > UINT16_MAX - reg_read(ind)) ? true : false;
        cpu.ticks += 4;
    }
    reg.F.bits.z = (reg_read(IND_A) == 0) ? true : false;
    reg.F.bits.h = ((reg_read(ind) & 0x0F) == 0x00);
}

static void SUB8(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(ind == IND_MEM_HL){
        reg_write(IND_A, (reg_read(IND_A) - ram_read8(reg_read(IND_HL))));
        reg.F.bits.cy = (reg_read(IND_A) > ram_read8(reg_read(IND_HL))) ? true : false;
        cpu.ticks += 8; 
    } 
    else{
        reg_write(IND_A, (reg_read(IND_A) - reg_read(ind)));
        reg.F.bits.cy = (reg_read(IND_A) > reg_read(ind)) ? true : false;
        cpu.ticks += 4;
    }
    reg.F.bits.z = (reg_read(IND_A) == 0) ? true : false;
    reg.F.bits.h = ((reg_read(ind) & 0x0F) == 0x00);
}

static void SUBC8(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(ind == IND_MEM_HL){
        reg_write(IND_A, (reg_read(IND_A) - (ram_read8(reg_read(IND_HL)  + reg.F.bits.cy))));
        reg.F.bits.cy = (reg_read(IND_A) > (ram_read8(reg_read(IND_HL) + reg.F.bits.cy))) ? true : false;
        cpu.ticks += 8; 
    } else{
        reg_write(IND_A, (reg_read(IND_A) - (reg_read(ind)  + reg.F.bits.cy)));
        reg.F.bits.cy = (reg_read(IND_A) > (reg_read(ind)  + reg.F.bits.cy)) ? true : false;
        cpu.ticks += 4;
    }
    reg.F.bits.z = (reg_read(IND_A) == 0) ? true : false;
    reg.F.bits.h = ((reg_read(ind) & 0x0F) == 0x00);
}

static void DI(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    printf("INTERRUPTS DISABLED\n");
    reg_interrupts_disabled_request = true;
    cpu.ticks += 4;
}

static void EI(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    printf("INTERRUPTS ENABLED\n");
    reg_interrupts_disabled_request = false;
    cpu.ticks += 4;
}

static void RST_GENERAL(cpu* cpu, uint16_t offset){
    reg_write(IND_SP, reg_read(IND_SP) - 2);
    ram_write16(reg_read(IND_SP), reg_read(IND_PC)+1);
    reg.PC_next = offset;
    cpu.ticks += 32;
}

static void RST00(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    RST_GENERAL(cpu, 0);
}

static void RST08(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    RST_GENERAL(cpu, 0x08);
}

static void RST10(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    RST_GENERAL(cpu, 0x10);
}

static void RST18(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    RST_GENERAL(cpu, 0x18);
}

static void RST20(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    RST_GENERAL(cpu, 0x20);
}

static void RST28(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    RST_GENERAL(cpu, 0x28);
}

static void RST30(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    RST_GENERAL(cpu, 0x30);
}

static void RST38(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    RST_GENERAL(cpu, 0x38);
}

static void CALL(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    reg_write(IND_PC, reg_read(IND_PC) + 3); //move PC 3 forward (next instruction)
    reg_write(IND_SP, reg_read(IND_SP) - 2); //move SP forward
    ram_write16(reg_read(IND_SP), reg_read(IND_PC));
    reg.PC_next = operand;
    cpu.ticks += 12;
}

static void CALLNZ(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(!reg.F.bits.z){
        reg_write(IND_PC, reg_read(IND_PC) + 3); //move PC 3 forward (next instruction)
        reg_write(IND_SP, reg_read(IND_SP) - 2); //move SP forward
        ram_write16(reg_read(IND_SP), reg_read(IND_PC));
        reg.PC_next = operand;
    }
        cpu.ticks += 12;
}

static void CALLZ(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(reg.F.bits.z){
        reg_write(IND_PC, reg_read(IND_PC) + 3); //move PC 3 forward (next instruction)
        reg_write(IND_SP, reg_read(IND_SP) - 2); //move SP forward
        ram_write16(reg_read(IND_SP), reg_read(IND_PC));
        reg.PC_next = operand;
    }
        cpu.ticks += 12;
}

static void RET(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    reg.PC_next = ram_read16(reg_read(IND_SP));
    reg_write(IND_SP, reg_read(IND_SP)+2);
    cpu.ticks += 8;
}

static void NOP(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    cpu.ticks += 4;
}

static void COMPL(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    reg_write(IND_A, ~reg_read(IND_A));
    cpu.ticks += 4;
}

static void SWAP(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(ind == IND_MEM_HL){

    } else{
        uint8_t old = reg_read(ind);
        uint8_t blah = ((old & 0xF0)>>4) + ((old & 0x0F)<<4);
        reg_write(ind, blah);
    }
    cpu.ticks += 8;
}

static void POP(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    reg_write(ind, ram_read16(reg_read(IND_SP)));
    reg_write(IND_SP, reg_read(IND_SP)+2);
    cpu.ticks += 12;
}

static void PUSH(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    reg_write(IND_SP, reg_read(IND_SP) - 2);
    ram_write16(reg_read(IND_SP), reg_read(ind));
    cpu.ticks += 20;
}

static void RETNZ(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(!reg.F.bits.z){
        reg.PC_next = ram_read16(reg_read(IND_SP));
        reg_write(IND_SP, reg_read(IND_SP)+2);
        cpu.ticks += 8;
    }
}

static void RETZ(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(reg.F.bits.z){
        reg.PC_next = ram_read16(reg_read(IND_SP));
        reg_write(IND_SP, reg_read(IND_SP)+2);
        cpu.ticks += 8;
    }
}

static void JRN(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    reg_write(IND_PC_NEXT, reg_read(IND_PC) + (operand & 0xFF));
    cpu.ticks += 8;
}

static void RES(uint16_t operand, cpu* cpu, reg_ind_t ind, reg_ind_t ind2){
    if(ind == IND_MEM_HL){
        ram_write8(reg_read(IND_HL), ram_read8(reg_read(IND_HL)) ^ 1 << (uint8_t(ind2) & 0x40));
    }
    else{
        reg_write(ind, reg_read(ind) ^ 1 << ((uint8_t)ind2 & 0x40));
    }
}



static opcode opcodes[256] = {
    {"NOP",               0, IND_NONE      , IND_NONE  , NOP}, //0x00
    {"LD BC,16b",         2, IND_BC        , IND_NONE  , LD16},
    {"LD (BC), A",        0, IND_BC        , IND_A     , LD_DIR_16},
    {"INC BC",            0, IND_BC        , IND_NONE  , INC},
    {"INC B",             0, IND_B         , IND_NONE  , INC},
    {"DEC B",             0, IND_B         , IND_NONE  , DEC},
    {"LD B,8b",           1, IND_B         , IND_NONE  , LD8 },
    {"RLCA",              0, IND_A         , IND_NONE  , RLC},
    {"LD SP,16b",         2, IND_SP        , IND_NONE  , LD16},
    {"ADD HL,BC",         0, IND_HL        , IND_BC    , ADD16},
    {"LD A,(BC)",         0, IND_NONE      , IND_NONE  , NULL},
    {"DEC BC",            0, IND_BC        , IND_NONE  , DEC},
    {"INC C",             0, IND_C         , IND_NONE  , INC},
    {"DEC C",             0, IND_C         , IND_NONE  , DEC},
    {"LD C,8b",           1, IND_C         , IND_NONE  , LD8},
    {"RRCA",              0, IND_A         , IND_NONE  , RRC},
    {"STOP 0",            1, IND_NONE      , IND_NONE  , NULL}, //0x10
    {"LD DE,16b",         2, IND_DE        , IND_NONE  , LD16},
    {"LD DE, A",          0, IND_DE        , IND_A     , LDMV},
    {"INC DE",            0, IND_DE        , IND_NONE  , INC},
    {"INC D",             0, IND_D         , IND_NONE  , INC},
    {"DEC D",             0, IND_D         , IND_NONE  , DEC},
    {"LD D 8b",           1, IND_D         , IND_NONE  , LD8},
    {"RLA",               0, IND_A         , IND_NONE  , RL},
    {"JR 8b",             1, IND_NONE      , IND_NONE  , JR},
    {"ADD HL,DE",         0, IND_HL        , IND_DE    , ADD16},
    {"LD A, DE",          0, IND_DE        , IND_NONE  , LDA_8_IND},
    {"DEC DE",            0, IND_DE        , IND_NONE  , DEC},
    {"INC E",             0, IND_E         , IND_NONE  , INC},
    {"DEC E",             0, IND_E         , IND_NONE  , DEC},
    {"LD E,8b",           1, IND_E         , IND_NONE  , LD8},
    {"RRA",               0, IND_A         , IND_NONE  , RR},
    {"JR NZ,8b",          1, IND_NONE      , IND_NONE  , JRNZ}, //0x20
    {"LD HL,16b",         2, IND_HL        , IND_NONE  , LD16},
    {"LD HL+,A",          0, IND_NONE      , IND_NONE  , LDA_HL_INC},
    {"INC HL",            0, IND_HL        , IND_NONE  , INC},
    {"INC H",             0, IND_H         , IND_NONE  , INC},
    {"DEC H",             0, IND_H         , IND_NONE  , DEC},
    {"LD H,8b",           1, IND_H         , IND_NONE  , LD8},
    {"DAA",               0, IND_NONE      , IND_NONE  , NULL},
    {"JR Z,8b",           1, IND_NONE      , IND_NONE  , JRZ},
    {"ADD HL,HL",         0, IND_HL        , IND_HL    , ADD16},
    {"LD A,HL+",          0, IND_A         , IND_NONE  , LD_HL_INC},
    {"DEC HL",            0, IND_HL        , IND_NONE  , DEC}, //indirected but register-local
    {"INC L",             0, IND_L         , IND_NONE  , INC},
    {"DEC L",             0, IND_L         , IND_NONE  , DEC},
    {"LD L,8b",           1, IND_L         , IND_NONE  , LD8},
    {"CPL",               0, IND_A         , IND_NONE  , COMPL},
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
    {"SBC A,B",           0, IND_B         , IND_NONE  , SUBC8},
    {"SBC A,C",           0, IND_C         , IND_NONE  , SUBC8},
    {"SBC A,D",           0, IND_D         , IND_NONE  , SUBC8},
    {"SBC A,E",           0, IND_E         , IND_NONE  , SUBC8},
    {"SBC A,H",           0, IND_H         , IND_NONE  , SUBC8},
    {"SBC A,L",           0, IND_L         , IND_NONE  , SUBC8},
    {"SBC A,HL",          0, IND_MEM_HL    , IND_NONE  , SUBC8},
    {"SBC A,A",           0, IND_A         , IND_NONE  , SUBC8},
    {"AND B",             0, IND_B         , IND_NONE  , AND}, //0xA0
    {"AND C",             0, IND_C         , IND_NONE  , AND},
    {"AND D",             0, IND_D         , IND_NONE  , AND},
    {"AND E",             0, IND_E         , IND_NONE  , AND},
    {"AND H",             0, IND_H         , IND_NONE  , AND},
    {"AND L",             0, IND_L         , IND_NONE  , AND},
    {"AND HL",            0, IND_MEM_HL    , IND_NONE  , AND},
    {"AND A",             0, IND_A         , IND_NONE  , AND},
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
    {"RET NZ",            0, IND_NONE      , IND_NONE  , RETNZ}, //0xC0
    {"POP BC",            0, IND_BC        , IND_NONE  , POP},
    {"JP NZ,16b",         2, IND_NONE      , IND_NONE  , JPNZ},
    {"JP 16b",            2, IND_NONE      , IND_NONE  , JUMP},
    {"CALL NZ,16b",       2, IND_NONE      , IND_NONE  , CALLNZ},
    {"PUSH BC",           0, IND_BC        , IND_NONE  , PUSH},
    {"ADD A,8b",          1, IND_NONE      , IND_NONE  , NULL},
    {"RST 00",            0, IND_NONE      , IND_NONE  , RST00},
    {"RET Z",             0, IND_NONE      , IND_NONE  , RETZ},
    {"RET",               0, IND_NONE      , IND_NONE  , RET},
    {"JP Z,16b",          0, IND_NONE      , IND_NONE  , NULL},
    {"PREFIX CB",         0, IND_NONE      , IND_NONE  , NULL},
    {"CALL Z,16b",        0, IND_NONE      , IND_NONE  , CALLZ},
    {"CALL 16b",          2, IND_NONE      , IND_NONE  , CALL},
    {"ADC A,8b",          0, IND_NONE      , IND_NONE  , NULL},
    {"RST 08",            0, IND_NONE      , IND_NONE  , RST08},
    {"RET NC",            0, IND_NONE      , IND_NONE  , NULL}, //0xD0
    {"POP DE",            0, IND_DE        , IND_NONE  , POP},
    {"JP NC,16b",         2, IND_NONE      , IND_NONE  , JPNZ},
    {"NULL INSTR",        0, IND_NONE      , IND_NONE  , NULL},
    {"CALL NC,16b",       2, IND_NONE      , IND_NONE  , NULL},
    {"PUSH DE",           0, IND_DE        , IND_NONE  , PUSH},
    {"SUB ?????,8b",      1, IND_NONE      , IND_NONE  , NULL},
    {"RST 10h",           0, IND_NONE      , IND_NONE  , RST10},
    {"RET C",             0, IND_NONE      , IND_NONE  , NULL},
    {"RET I",             0, IND_NONE      , IND_NONE  , NULL},
    {"JP C,16b",          2, IND_NONE      , IND_NONE  , NULL},
    {"NULL INSTR",        0, IND_NONE      , IND_NONE  , NULL},
    {"CALL C,16b",        2, IND_NONE      , IND_NONE  , NULL},
    {"NULL INSTR",        0, IND_NONE      , IND_NONE  , NOP},
    {"SBC A,8b",          1, IND_NONE      , IND_NONE  , NULL},
    {"RST 18h",           0, IND_NONE      , IND_NONE  , RST18},
    {"LDH 8b, A",         1, IND_A         , IND_NONE  , LDDIR}, //0xE0
    {"POP HL",            0, IND_HL        , IND_NONE  , POP},
    {"LD (C), A",         0, IND_C         , IND_A     , LDDIR},
    {"NULL INSTR",        0, IND_NONE      , IND_NONE  , NULL},
    {"NULL INSTR",        0, IND_NONE      , IND_NONE  , NULL},
    {"PUSH HL",           0, IND_HL        , IND_NONE  , PUSH},
    {"AND 8b",            1, IND_NONE      , IND_NONE  , AND},
    {"RST 20h",           0, IND_NONE      , IND_NONE  , RST20},
    {"ADD SP, 8b",        1, IND_NONE      , IND_NONE  , NULL},
    {"JP (HL)",           0, IND_MEM_HL    , IND_NONE  , JUMP},
    {"LD 16b, A",         2, IND_NONE      , IND_A     , LD16},
    {"NULL INSTR",        0, IND_NONE      , IND_NONE  , NULL},
    {"NULL INSTR",        0, IND_NONE      , IND_NONE  , NULL},
    {"NULL INSTR",        0, IND_NONE      , IND_NONE  , NULL},
    {"XOR 8b",            1, IND_NONE      , IND_NONE  , NULL},
    {"RST 28h",           0, IND_NONE      , IND_NONE  , RST28},
    {"LDH A,(8b)",        1, IND_NONE      , IND_A     , LDDIR}, //0xF0
    {"POP AF",            0, IND_AF        , IND_NONE  , POP},
    {"LD A,(C)",          1, IND_NONE      , IND_NONE  , NULL},
    {"DI",                0, IND_NONE      , IND_NONE  , DI},
    {"NULL INSTR",        0, IND_NONE      , IND_NONE  , NULL},
    {"PUSH AF",           0, IND_AF        , IND_NONE  , PUSH},
    {"NULL INSTR",        0, IND_NONE      , IND_NONE  , NULL},
    {"NULL INSTR",        0, IND_NONE      , IND_NONE  , NULL},
    {"NULL INSTR",        0, IND_NONE      , IND_NONE  , NULL},
    {"NULL INSTR",        0, IND_NONE      , IND_NONE  , NULL},
    {"LD a,16b",          2, IND_NONE      , IND_A     , LDMV},
    {"EI",                0, IND_NONE      , IND_NONE  , EI},
    {"NULL INSTR",        0, IND_NONE      , IND_NONE  , NULL},
    {"NULL INSTR",        0, IND_NONE      , IND_NONE  , NULL},
    {"CP 8b",             1, IND_NONE      , IND_NONE  , CP  },
    {"RST 38h",           0, IND_NONE      , IND_NONE  , RST38},
};

static opcode aux_opcodes[256] = {
    {"RLC B",             0, IND_B         , IND_NONE     , RLC}, //0x00
    {"RLC C",             0, IND_C         , IND_NONE     , RLC},
    {"RLC D",             0, IND_D         , IND_NONE     , RLC},
    {"RLC E",             0, IND_E         , IND_NONE     , RLC},
    {"RLC H",             0, IND_H         , IND_NONE     , RLC},
    {"RLC L",             0, IND_L         , IND_NONE     , RLC},
    {"RLC (HL)",          0, IND_MEM_HL    , IND_NONE     , RLC},
    {"RLC A",             0, IND_A         , IND_NONE     , RLC},
    {"RRC B",             0, IND_B         , IND_NONE     , RRC},
    {"RRC C",             0, IND_C         , IND_NONE     , RRC},
    {"RRC D",             0, IND_D         , IND_NONE     , RRC},
    {"RRC E",             0, IND_E         , IND_NONE     , RRC},
    {"RRC H",             0, IND_H         , IND_NONE     , RRC},
    {"RRC L",             0, IND_L         , IND_NONE     , RRC},
    {"RRC (HL)",          0, IND_MEM_HL    , IND_NONE     , RRC},
    {"RRC A",             0, IND_A         , IND_NONE     , RRC},
    {"RL B",              0, IND_B         , IND_NONE     , RL}, //0x10
    {"RL C",              0, IND_C         , IND_NONE     , RL},
    {"RL D",              0, IND_D         , IND_NONE     , RL},
    {"RL E",              0, IND_E         , IND_NONE     , RL},
    {"RL H",              0, IND_H         , IND_NONE     , RL},
    {"RL L",              0, IND_L         , IND_NONE     , RL},
    {"RL (HL)",           0, IND_MEM_HL    , IND_NONE     , RL},
    {"RL A",              0, IND_A         , IND_NONE     , RL},
    {"RR B",              0, IND_B         , IND_NONE     , RR},
    {"RR C",              0, IND_C         , IND_NONE     , RR},
    {"RR D",              0, IND_D         , IND_NONE     , RR},
    {"RR E",              0, IND_E         , IND_NONE     , RR},
    {"RR H",              0, IND_H         , IND_NONE     , RR},
    {"RR L",              0, IND_L         , IND_NONE     , RR},
    {"RR (HL)",           0, IND_MEM_HL    , IND_NONE     , RR},
    {"RR A",              0, IND_A         , IND_NONE     , RR},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL}, //0x20
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL}, //0x30
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"SWAP A",            0, IND_A         , IND_NONE     , SWAP},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL}, //0x40
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL}, //0x50
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL}, //0x60
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL}, //0x70
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"NOT IMPL",          0, IND_NONE      , IND_NONE     , NULL},
    {"RES 0 B",           0, IND_B         , (reg_ind_t)0 , RES}, //0x80
    {"RES 0 C",           0, IND_C         , (reg_ind_t)0 , RES},
    {"RES 0 D",           0, IND_D         , (reg_ind_t)0 , RES},
    {"RES 0 E",           0, IND_E         , (reg_ind_t)0 , RES},
    {"RES 0 H",           0, IND_H         , (reg_ind_t)0 , RES},
    {"RES 0 L",           0, IND_L         , (reg_ind_t)0 , RES},
    {"RES 0 HL",          0, IND_MEM_HL    , (reg_ind_t)0 , RES},
    {"RES 0 A",           0, IND_A         , (reg_ind_t)0 , RES},
    {"RES 1 B",           0, IND_B         , (reg_ind_t)1 , RES},
    {"RES 1 C",           0, IND_C         , (reg_ind_t)1 , RES},
    {"RES 1 D",           0, IND_D         , (reg_ind_t)1 , RES},
    {"RES 1 E",           0, IND_E         , (reg_ind_t)1 , RES},
    {"RES 1 H",           0, IND_H         , (reg_ind_t)1 , RES},
    {"RES 1 L",           0, IND_L         , (reg_ind_t)1 , RES},
    {"RES 1 HL",          0, IND_MEM_HL    , (reg_ind_t)1 , RES},
    {"RES 1 A",           0, IND_A         , (reg_ind_t)1 , RES},
    {"RES 2 B",           0, IND_B         , (reg_ind_t)2 , RES}, //0x90
    {"RES 2 C",           0, IND_C         , (reg_ind_t)2 , RES},
    {"RES 2 D",           0, IND_D         , (reg_ind_t)2 , RES},
    {"RES 2 E",           0, IND_E         , (reg_ind_t)2 , RES},
    {"RES 2 H",           0, IND_H         , (reg_ind_t)2 , RES},
    {"RES 2 L",           0, IND_L         , (reg_ind_t)2 , RES},
    {"RES 2 HL",          0, IND_MEM_HL    , (reg_ind_t)2 , RES},
    {"RES 2 A",           0, IND_A         , (reg_ind_t)2 , RES},
    {"RES 3 B",           0, IND_B         , (reg_ind_t)3 , RES},
    {"RES 3 C",           0, IND_C         , (reg_ind_t)3 , RES},
    {"RES 3 D",           0, IND_D         , (reg_ind_t)3 , RES},
    {"RES 3 E",           0, IND_E         , (reg_ind_t)3 , RES},
    {"RES 3 H",           0, IND_H         , (reg_ind_t)3 , RES},
    {"RES 3 L",           0, IND_L         , (reg_ind_t)3 , RES},
    {"RES 3 HL",          0, IND_MEM_HL    , (reg_ind_t)3 , RES},
    {"RES 3 A",           0, IND_A         , (reg_ind_t)3 , RES},
    {"RES 4 B",           0, IND_B         , (reg_ind_t)4 , RES}, //0xA0
    {"RES 4 C",           0, IND_C         , (reg_ind_t)4 , RES},
    {"RES 4 D",           0, IND_D         , (reg_ind_t)4 , RES},
    {"RES 4 E",           0, IND_E         , (reg_ind_t)4 , RES},
    {"RES 4 H",           0, IND_H         , (reg_ind_t)4 , RES},
    {"RES 4 L",           0, IND_L         , (reg_ind_t)4 , RES},
    {"RES 4 HL",          0, IND_MEM_HL    , (reg_ind_t)4 , RES},
    {"RES 4 A",           0, IND_A         , (reg_ind_t)4 , RES},
    {"RES 5 B",           0, IND_B         , (reg_ind_t)5 , RES},
    {"RES 5 C",           0, IND_C         , (reg_ind_t)5 , RES},
    {"RES 5 D",           0, IND_D         , (reg_ind_t)5 , RES},
    {"RES 5 E",           0, IND_E         , (reg_ind_t)5 , RES},
    {"RES 5 H",           0, IND_H         , (reg_ind_t)5 , RES},
    {"RES 5 L",           0, IND_L         , (reg_ind_t)5 , RES},
    {"RES 5 HL",          0, IND_MEM_HL    , (reg_ind_t)5 , RES},
    {"RES 5 A",           0, IND_A         , (reg_ind_t)5 , RES},
    {"RES 6 B",           0, IND_B         , (reg_ind_t)6 , RES}, //0xB0
    {"RES 6 C",           0, IND_C         , (reg_ind_t)6 , RES},
    {"RES 6 D",           0, IND_D         , (reg_ind_t)6 , RES},
    {"RES 6 E",           0, IND_E         , (reg_ind_t)6 , RES},
    {"RES 6 H",           0, IND_H         , (reg_ind_t)6 , RES},
    {"RES 6 L",           0, IND_L         , (reg_ind_t)6 , RES},
    {"RES 6 HL",          0, IND_MEM_HL    , (reg_ind_t)6 , RES},
    {"RES 6 A",           0, IND_A         , (reg_ind_t)6 , RES},
    {"RES 7 B",           0, IND_B         , (reg_ind_t)7 , RES},
    {"RES 7 C",           0, IND_C         , (reg_ind_t)7 , RES},
    {"RES 7 D",           0, IND_D         , (reg_ind_t)7 , RES},
    {"RES 7 E",           0, IND_E         , (reg_ind_t)7 , RES},
    {"RES 7 H",           0, IND_H         , (reg_ind_t)7 , RES},
    {"RES 7 L",           0, IND_L         , (reg_ind_t)7 , RES},
    {"RES 7 HL",          0, IND_MEM_HL    , (reg_ind_t)7 , RES},
    {"RES 7 A",           0, IND_A         , (reg_ind_t)7 , RES},
    {"EURHG?",            0, IND_B         , (reg_ind_t)1 , NULL},




};

cpu_init(){
    reg.PC = 0;
    ticks = 0;
    inter.set_ram_ptr(&ram);
    ren.set_ram(&ram);
}

uint16_t find_16_from_8(uint8_t* array, int ptr){
    uint8_t low = array[ptr];
    uint8_t high = array[ptr+1];
    uint16_t combined = (uint16_t)high;
    combined = combined << 8;
    combined = combined + low;
    return combined;
}

uint16_t cpu_get_operand(opcode ctx){
    if(ctx.operand_length == 1){
        return(ram.read8(reg.read(IND_PC)+1));
    } else if(ctx.operand_length == 2){
        return(ram.read16(reg.read(IND_PC)+1));
    } else{
        return(0);
    }
}


int cpu_execute_opcode(){
    inter.service(ticks);

    if(ram.read8(reg.read(IND_PC)) == 0xCB){
        reg.write(IND_PC, reg.read(IND_PC)+1);
        return(execute_aux_opcode());
    }
    opcode ctx = opcodes[ram.read8(reg.read(IND_PC))];
    uint16_t operand = get_operand(ctx);
    // printf("Ticks: %d\n", (int)ticks);
    printf("PC: 0x%04X [0x%02X]:(%s) - {0x%04X}\n",reg.PC, ram.read8(reg.read(IND_PC)), ctx.str_name, operand);

    
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

static int cpu_execute_aux_opcode(){
    opcode aux_ctx = aux_opcodes[ram.read8(reg.read(IND_PC))];
    uint16_t operand = get_operand(aux_ctx);
    // printf("Aux\n");
    // printf("Ticks: %d\n", (int)ticks);
    printf("PC: 0x%04X aux[0x%02X]:(%s) - {0x%04X}\n",reg.PC, ram.read8(reg.read(IND_PC)), aux_ctx.str_name, operand);

    
    if(aux_ctx.opcode_pointer == NULL){
        printf("AUX instruction not implemented :(\n");
        return(-1);
    } else{
        reg.PC_next = reg.PC + aux_ctx.operand_length + 1;
        aux_ctx.opcode_pointer(operand,this, aux_ctx.reg_ind, aux_ctx.reg_ind_two);
        reg.PC_step();
        return 0;
    }
}

void cpu_dump_registers(){
    printf("Register dump\n");
    printf("A: 0x%02X F: 0x%02X ", reg.A, reg.F.byte);
    printf("B: 0x%02X C: 0x%02X\n", reg.B, reg.C);
    printf("D: 0x%02X E: 0x%02X ", reg.D, reg.E);
    printf("H: 0x%02X L: 0x%02X\n", reg.H, reg.L);
    printf("PC: 0x%04X ", reg.PC);
    printf("SP: 0x%04X, pointing at 0x%04X\n", reg.SP, ram.read16(reg.SP));
    printf("FLAGS:\n z; %d | n: %d | h: %d | cy: %d \n\n",
    reg.F.bits.z, reg.F.bits.n, reg.F.bits.h, reg.F.bits.cy);
}

void cpu_open_file(const char* f_addr){
    FILE* rom_ptr;
    rom_ptr = fopen(f_addr, "rb");
    if(rom_ptr == NULL){
        printf("file not opened :(\n");
        exit(-1);
    }
    fseek(rom_ptr, 0, SEEK_END);
    int rom_size = ftell(rom_ptr);
    printf("rom size: %d\n", rom_size);
    fseek(rom_ptr, 0, SEEK_SET);

    uint8_t* rom = (uint8_t*)malloc(rom_size);
    fread(rom, 1, rom_size, rom_ptr);
    ram.copy_cartridge(rom);
    fclose(rom_ptr);
}

void cpu_dump_stack(){
    printf("~~ Stack pointer: 0x%04X ~~\n", reg.SP);
    for(int sp_t = 0xcfff; sp_t >= reg.SP; sp_t-=2){
        printf("0x%04X: 0X%04X\n", sp_t, ram.read16(sp_t));
    }
}