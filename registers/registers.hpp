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
    REG_IND_A,
    REG_IND_B,
    REG_IND_C,
    REG_IND_D,
    REG_IND_E,
    REG_IND_F,
    REG_IND_H,
    REG_IND_L,
    REG_IND_BC,
    REG_IND_DE,
    REG_IND_HL,
    REG_IND_SP,
    REG_IND_PC,
    REG_IND_MEM_HL,
    REG_IND_PC_NEXT,
    REG_IND_NONE,
} reg_ind_t;

class registers{
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

        uint16_t read(reg_ind_t);
        void write(reg_ind_t, uint16_t);
        void PC_step();
        uint16_t PC_next;
};

typedef struct{
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
} registers_t;




#endif /* REGISTERS_HPP */