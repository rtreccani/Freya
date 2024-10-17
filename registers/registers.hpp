#ifndef REGISTERS_HPP
#define REGISTERS_HPP

#include <stdbool.h>
#include <stdint.h>

//used for the flags in the CPU F register
//as a packed struct
typedef struct {
    bool z:1;
    bool n:1;
    bool h:1;
    bool cy:1;
} flag_reg;

typedef union {
    flag_reg bits;
    uint8_t byte;
} flag_reg_t;

typedef enum{
    IND_A,
    IND_B,
    IND_C,
    IND_D,
    IND_E,
    IND_F,
    IND_H,
    IND_L,
    IND_AF,
    IND_BC,
    IND_DE,
    IND_HL,
    IND_SP,
    IND_PC,
    IND_MEM_HL,
    IND_MEM_DE,
    IND_MEM_BC,
    IND_PC_NEXT,
    IND_NONE,
} reg_ind_t;

typedef struct registers{
    public:
        uint8_t A;
        flag_reg_t F;

        union{
            struct{
                uint8_t C;
                uint8_t B;
            };
            uint16_t BC;
        };

        union{
            struct{
                uint8_t E;
                uint8_t D;
            };
            uint16_t DE;
        };

        union{
            struct{
                uint8_t L;
                uint8_t H;
            };
            uint16_t HL;
        };

        uint16_t PC;
        uint16_t SP;

        uint16_t PC_next;
        bool interrupts_enabled;
        bool interrupts_disabled_request;
} registers_t;

uint16_t reg_read(reg_ind_t);
void reg_write(reg_ind_t, uint16_t);
void reg_PC_step();

#endif /* REGISTERS_HPP */