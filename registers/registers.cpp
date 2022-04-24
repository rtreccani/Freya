#include <registers.hpp>
#include <stdio.h>

void registers::write(reg_ind_t ind, uint16_t val){
    switch(ind){
//8 bit writes
        case IND_A:
            A = (val & 0xFF);
            break;
        case IND_B:
            B = (val & 0xFF);
            break;
        case IND_C:
            C = (val & 0xFF);
            break;
        case IND_D:
            D = (val & 0xFF);
            break;
        case IND_E:
            E = (val & 0xFF);
            break;
        case IND_H:
            H = (val & 0xFF);
            break;
        case IND_L:
            L = (val & 0xFF);
            break;
// 16 bit writes
        case IND_AF:
            A = val & 0xFF00;
            F.byte = val & 0x00FF;
        case IND_BC:
            BC = val;
            break;
        case IND_DE:
            DE = val;
            break;
        case IND_HL:
            HL = val;
            break;
        case IND_SP:
            SP = val;
            break;
        case IND_PC:
            PC = val;
            break;
        case IND_PC_NEXT:
            PC_next = val;
            break;
    }
}

uint16_t registers::read(reg_ind_t ind){
    switch(ind){
//8 bit reads
        case IND_A:
            return(A & 0xFF);
        case IND_B:
            return(B & 0xFF);
        case IND_C:
            return(C & 0xFF);
        case IND_D:
            return(D & 0xFF);
        case IND_E:
            return(E & 0xFF);
        case IND_H:
            return(H & 0xFF);
        case IND_L:
            return(L & 0xFF);
// 16 bit reads
        case IND_AF:
            return ((A << 8) + F.byte);
        case IND_BC:
            return(BC);
        case IND_DE:
            return(DE);
        case IND_HL:
            return(HL);
        case IND_SP:
            return(SP);
        case IND_PC:
            return(PC);
        case IND_PC_NEXT:
            return(PC_next);
        
    }
    return(0);
}

void registers::PC_step(){
    PC = PC_next;
}