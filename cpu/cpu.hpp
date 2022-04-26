#ifndef CPU_H
#define CPU_H

#include <registers.hpp>
#include <interrupts.hpp>
#include <ram.hpp>
#include <renderer.hpp>
#include <stddef.h>

#define M_RAM_0 0xC000

struct opcode;


class cpu{
    public:
        void open_file(const char*);
        void dump_stack();
        RAM ram;
        registers reg;
        interrupts inter;
        renderer ren;
        uint8_t* p_rom;
        cpu();
        int execute_opcode();
        int execute_aux_opcode();
        void jump(int);
        void dump_registers();
        uint16_t get_operand(opcode);
        uint16_t get_aux_operand(opcode);
        uint64_t ticks;
};

struct opcode{
    const char* str_name;
    int operand_length;
    reg_ind_t reg_ind;
    reg_ind_t reg_ind_two;
    void(* opcode_pointer)(uint16_t, cpu*, reg_ind_t, reg_ind_t);
};

#endif /*CPU_H*/