#include "instr.h"

vm_instr_t vm_decode_instr(uint16_t data) {
    vm_instr_t instr = {
        .size = (data >> 14) & 0b11,
        .opcode = (data >> 8) & 0b111111,
        .src = (data >> 6) & 0b11,
        .dst = (data >> 4) & 0b11,
        .flags = data & 0b1111
    };
    return instr;
}
