#include <registers.hpp>
#include <stdio.h>

registers_t reg;

void reg_write(reg_ind_t ind, uint16_t val){
    switch(ind){
//8 bit writes
        case IND_A:
            reg.A = (val & 0xFF);
            break;
        case IND_B:
            reg.B = (val & 0xFF);
            break;
        case IND_C:
            reg.C = (val & 0xFF);
            break;
        case IND_D:
            reg.D = (val & 0xFF);
            break;
        case IND_E:
            reg.E = (val & 0xFF);
            break;
        case IND_H:
            reg.H = (val & 0xFF);
            break;
        case IND_L:
            reg.L = (val & 0xFF);
            break;
// 16 bit writes
        case IND_AF:
            reg.A = val & 0xFF00;
            reg.F.byte = val & 0x00FF;
        case IND_BC:
            reg.BC = val;
            break;
        case IND_DE:
            reg.DE = val;
            break;
        case IND_HL:
            reg.HL = val;
            break;
        case IND_SP:
            reg.SP = val;
            break;
        case IND_PC:
            reg.PC = val;
            break;
        case IND_PC_NEXT:
            reg.PC_next = val;
            break;

        default:
            break;
    }
}

uint16_t reg_read(reg_ind_t ind){
    switch(ind){
//8 bit reads
        case IND_A:
            return(reg.A & 0xFF);
        case IND_B:
            return(reg.B & 0xFF);
        case IND_C:
            return(reg.C & 0xFF);
        case IND_D:
            return(reg.D & 0xFF);
        case IND_E:
            return(reg.E & 0xFF);
        case IND_H:
            return(reg.H & 0xFF);
        case IND_L:
            return(reg.L & 0xFF);
// 16 bit reads
        case IND_AF:
            return ((reg.A << 8) + reg.F.byte);
        case IND_BC:
            return(reg.BC);
        case IND_DE:
            return(reg.DE);
        case IND_HL:
            return(reg.HL);
        case IND_SP:
            return(reg.SP);
        case IND_PC:
            return(reg.PC);
        case IND_PC_NEXT:
            return(reg.PC_next);
        default:
            break;
    }
    return(0);
}

void reg_PC_step(){
    reg.PC = reg.PC_next;
}