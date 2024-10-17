#ifndef RAM_H
#define RAM_H

#include <stdint.h>

#define ROM_0_START  0x0000
#define ROM_0_END    0x3FFF

#define ROM_SW_START 0x4000
#define ROM_SW_END   0x7FFF

#define VRAM_START   0x8000
#define VRAM_END     0x9FFF

#define RAM_SW_START 0xA000
#define RAM_SW_END   0xBFFF

#define RAM_0_START 0xC000
#define RAM_0_END   0xDFFF

#define RAM_0_ECHO_START 0xE000
#define RAM_0_ECHO_END   0xFDFF

#define SPRITE_ATTR_START 0xFE00
#define SPRITE_ATTR_END   0xFE9F

#define EMPTY_IO_START 0xFEA0
#define EMPTY_IO_END   0xFEFF

#define IO_START 0xFF00
#define IO_END   0xFF4B

#define EMPTY_IO_2_START 0xFF4C
#define EMPTY_IO_2_END   0xFF7F

#define INTERNAL_RAM_2_START 0xFF80
#define INTERNAL_RAM_2_END   0xFFFE

#define INTERRUPT_EN_REGISTER 0xFFFF

typedef enum {
    ROM_0,
    ROM_SW,
    VRAM,
    RAM_SW,
    RAM_0,
    RAM_0_ECHO,
    SPRITE_ATTR,
    EMPTY_IO,
    IO,
    EMPTY_IO_2,
    INTERNAL_RAM_2,
    INTERRUPT_EN,
} ram_section_t;

typedef struct{
    uint16_t native_address;
    const char* friendly_name;
    void(* p_special_function_write)(uint16_t, uint8_t);
    void(* p_special_function_read)(uint16_t, uint8_t);
} special_memory_t;

typedef struct{
    ram_section_t section;
    uint16_t address;
} ram_map_t;

#define M_RAM_S 0xC000
#define M_RAM_END 0xFFFF

uint8_t ram_read8(uint16_t);
uint16_t ram_read16(uint16_t); //for getting operands
void ram_write16(uint16_t, uint16_t); //for SP
void ram_write8(uint16_t, uint8_t);
void ram_inc(uint16_t);
void ram_dec(uint16_t);
void ram_add(uint16_t, uint8_t);
void ram_sub(uint16_t, uint8_t);
void ram_copy_cartridge(uint8_t *);


typedef struct{
    uint8_t VRAM_data[VRAM_END - VRAM_START];
    uint8_t ROM_0_data[ROM_0_END - ROM_0_START];
    uint8_t ROM_SW_data[ROM_SW_END - ROM_SW_START];

    uint8_t RAM_SW_data[RAM_SW_END - RAM_SW_START];
    uint8_t RAM_0_data[RAM_0_END - RAM_0_START];
    uint8_t SPRITE_ATTR_data[SPRITE_ATTR_END - SPRITE_ATTR_START];
    uint8_t IO_data[IO_END - IO_START];
    uint8_t INTERNAL_RAM_2_data[INTERNAL_RAM_2_END - INTERNAL_RAM_2_START];
    uint8_t INTERRUPT_EN_data;
} ram_t;

#endif