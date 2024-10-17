#ifndef CPU_H
#define CPU_H

#include <registers.hpp>
#include <interrupts.hpp>
#include <ram.hpp>
#include <renderer.hpp>
#include <stddef.h>

struct opcode;


void cpu_open_file(const char*);
void cpu_dump_stack();
void cpu_init();
int cpu_execute_opcode();
static int cpu_execute_aux_opcode();
void jump(int);
void dump_registers();
uint16_t cpu_get_operand(opcode);
uint16_t cpu_get_aux_operand(opcode);


typedef struct{
    uint64_t ticks;
} cpu_t;

struct opcode{
    const char* str_name;
    int operand_length;
    reg_ind_t reg_ind;
    reg_ind_t reg_ind_two;
    void(* opcode_pointer)(uint16_t, cpu*, reg_ind_t, reg_ind_t);
};

#endif /*CPU_H*/