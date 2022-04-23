#include <registers.hpp>
#include <stdio.h>

void registers::write(reg_ind_t ind, uint16_t val){
    switch(ind){
//8 bit writes
        case REG_IND_A:
            A = (val & 0xFF);
            break;
        case REG_IND_B:
            B = (val & 0xFF);
            break;
        case REG_IND_C:
            C = (val & 0xFF);
            break;
        case REG_IND_D:
            D = (val & 0xFF);
            break;
        case REG_IND_E:
            A = (val & 0xFF);
            break;
        case REG_IND_H:
            H = (val & 0xFF);
            break;
        case REG_IND_L:
            L = (val & 0xFF);
            break;
// 16 bit writes
        case REG_IND_BC:
            BC = val;
            break;
        case REG_IND_DE:
            DE = (val & 0xFF);
            break;
        case REG_IND_HL:
            HL = val;
            break;
        case REG_IND_SP:
            SP = val;
            break;
        case REG_IND_PC:
            PC = val;
            break;
        case REG_IND_PC_NEXT:
            PC_next = val;
            break;
    }
}

uint16_t registers::read(reg_ind_t ind){
    switch(ind){
//8 bit reads
        case REG_IND_A:
            return(A & 0xFF);
        case REG_IND_B:
            return(B & 0xFF);
        case REG_IND_C:
            return(C & 0xFF);
        case REG_IND_D:
            return(D & 0xFF);
        case REG_IND_E:
            return(E & 0xFF);
        case REG_IND_H:
            return(H & 0xFF);
        case REG_IND_L:
            return(L & 0xFF);
// 8 bit writes
        case REG_IND_BC:
            return(BC);
        case REG_IND_DE:
            return(DE);
        case REG_IND_HL:
            return(HL);
        case REG_IND_SP:
            return(SP);
        case REG_IND_PC:
            return(PC);
        case REG_IND_PC_NEXT:
            return(PC_next);
        
    }
    return(0);
}

void registers::PC_step(){
    PC = PC_next;
}