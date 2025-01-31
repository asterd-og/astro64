#pragma once

#include <stdint.h>
#include <stddef.h>

#define INSTR_OFF_SRC     0b0001
#define INSTR_OFF_DST     0b0010
#define INSTR_REG_OFF_SRC 0b0100
#define INSTR_REG_OFF_DST 0b1000

#define JMP_REL     0b00000001
#define JMP_CARRY   0b00000010
#define JMP_ZERO    0b00000100
#define JMP_GREATER 0b00001000
#define JMP_LESSER  0b00010000

typedef struct {
    uint8_t size : 2;
    uint8_t opcode : 6;
    uint8_t src : 2;
    uint8_t dst : 2;
    uint8_t flags : 4;
} vm_instr_t;

vm_instr_t vm_decode_instr(uint16_t data);
