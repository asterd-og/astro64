#include "cpu.h"
#include "mem.h"
#include "ivt.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

vm_t *vm_create(size_t ram_size) {
    vm_t *vm = (vm_t*)malloc(sizeof(vm_t));
    memset(vm, 0, sizeof(vm_t));
    vm->ram = (uint8_t*)malloc(ram_size);
    vm->ram_size = ram_size;
    memset(vm->ram, 0, ram_size);
    return vm;
}

void vm_load_rom(vm_t *vm, uint8_t *rom, size_t rom_size) {
    if (rom_size > 4096) {
        vm->halted = true;
        return;
    }
    memcpy(vm->ram, rom, rom_size);
}

uint32_t vm_clock(vm_t *vm) {
    vm_run_queued_int(vm);
    vm->cycles = 0;
    uint16_t fetch = vm_read16(vm, vm->registers[IP]);
    vm->registers[IP] += 2;
    vm_instr_t instr = vm_decode_instr(fetch);
    vm->cycles++; // Decoding takes 1 cycle.
    vm_op_lookup[instr.opcode](vm, &instr); // Operation should update cycles accordingly.
    return vm->cycles;
}

void vm_push8(vm_t *vm, uint8_t data) {
    vm->registers[SP] -= 1;
    vm_write8(vm, vm->registers[SP], data);
}

void vm_push16(vm_t *vm, uint16_t data) {
    vm->registers[SP] -= 2;
    vm_write16(vm, vm->registers[SP], data);
}

void vm_push32(vm_t *vm, uint32_t data) {
    vm->registers[SP] -= 4;
    vm_write32(vm, vm->registers[SP], data);
}

void vm_push64(vm_t *vm, uint64_t data) {
    vm->registers[SP] -= 8;
    vm_write64(vm, vm->registers[SP], data);
}

uint8_t vm_pop8(vm_t *vm) {
    uint8_t data = vm_read8(vm, vm->registers[SP]);
    vm->registers[SP] += 1;
    return data;
}

uint16_t vm_pop16(vm_t *vm) {
    uint16_t data = vm_read16(vm, vm->registers[SP]);
    vm->registers[SP] += 2;
    return data;
}

uint32_t vm_pop32(vm_t *vm) {
    uint32_t data = vm_read32(vm, vm->registers[SP]);
    vm->registers[SP] += 4;
    return data;
}

uint64_t vm_pop64(vm_t *vm) {
    uint64_t data = vm_read64(vm, vm->registers[SP]);
    vm->registers[SP] += 8;
    return data;
}

void *vm_src_ptr(vm_t *vm, vm_instr_t *instr) {
    uint64_t src_off = 0;
    if (instr->flags & INSTR_OFF_SRC) {
        if (instr->flags & INSTR_REG_OFF_SRC) {
            // Reg
            uint8_t reg = vm_read8(vm, vm->registers[IP]);
            vm->registers[IP]++;
            src_off = vm->registers[reg];
        } else {
            // IMM
            src_off = vm_read64(vm, vm->registers[IP]);
            vm->registers[IP] += 8;
        }
    }
    void *ptr = NULL;
    switch (instr->src) {
    case 0: {
        // Reg
        uint8_t reg = vm_read8(vm, vm->registers[IP]);
        vm->registers[IP]++;
        ptr = &vm->registers[reg];
        break;
    }
    case 1: {
        // [Reg]
        uint8_t reg = vm_read8(vm, vm->registers[IP]);
        vm->registers[IP]++;
        uint64_t address = vm->registers[reg];
        address += src_off;
        ptr = vm_ptr(vm, address);
        break;
    }
    case 2: {
        // IMM
        ptr = vm_ptr(vm, vm->registers[IP]);
        vm->registers[IP] += SZ(instr->size);
        vm->cycles++;
        break;
    }
    case 3: {
        // [IMM]
        uint64_t address = vm_read64(vm, vm->registers[IP]);
        vm->registers[IP] += 8;
        address += src_off;
        ptr = vm_ptr(vm, address);
        break;
    }
    }
    return ptr;
}

void *vm_dst_ptr(vm_t *vm, vm_instr_t *instr) {
    uint64_t dst_off = 0;
    if (instr->flags & INSTR_OFF_DST) {
        if (instr->flags & INSTR_REG_OFF_DST) {
            // Reg
            uint8_t reg = vm_read8(vm, vm->registers[IP]);
            vm->registers[IP]++;
            dst_off = vm->registers[reg];
        } else {
            // IMM
            dst_off = vm_read64(vm, vm->registers[IP]);
            vm->registers[IP] += 8;
        }
    }
    void *ptr = NULL;
    switch (instr->dst) {
    case 0: {
        // Reg
        uint8_t reg = vm_read8(vm, vm->registers[IP]);
        vm->registers[IP]++;
        ptr = &vm->registers[reg];
        break;
    }
    case 1: {
        // [Reg]
        uint8_t reg = vm_read8(vm, vm->registers[IP]);
        vm->registers[IP]++;
        uint64_t address = vm->registers[reg];
        address += dst_off;
        ptr = vm_ptr(vm, address);
        break;
    }
    case 2: {
        // [IMM]
        uint64_t address = vm_read64(vm, vm->registers[IP]);
        vm->registers[IP] += 8;
        address += dst_off;
        ptr = vm_ptr(vm, address);
        break;
    }
    }
    return ptr;
}

void vm_nop(vm_t *vm, vm_instr_t *instr) {
}

#define ARITH_OP(vm, instr, carry, zero, type, op) \
    type *src = (type*)vm_src_ptr(vm, instr); \
    type *dst = (type*)vm_dst_ptr(vm, instr); \
    type res = 0; \
    carry = op(*dst, *src, &res); \
    vm->cycles++; /* doing an operation takes a cycle */ \
    *dst = res; \
    zero = res == 0

#define SET_FLAGS(vm, carry, zero) \
    if (carry) vm->registers[FLAGS] |= F_CARRY; \
    else vm->registers[FLAGS] &= ~F_CARRY; \
    if (zero) vm->registers[FLAGS] |= F_ZERO; \
    else vm->registers[FLAGS] &= ~F_CARRY

void vm_add(vm_t *vm, vm_instr_t *instr) {
    bool carry;
    bool zero;
    switch (instr->size) {
    case 0: { ARITH_OP(vm, instr, carry, zero, uint8_t, __builtin_add_overflow); break; }
    case 1: { ARITH_OP(vm, instr, carry, zero, uint16_t, __builtin_add_overflow); break; }
    case 2: { ARITH_OP(vm, instr, carry, zero, uint32_t, __builtin_add_overflow); break; }
    case 3: { ARITH_OP(vm, instr, carry, zero, uint64_t, __builtin_add_overflow); break; }
    }
    SET_FLAGS(vm, carry, zero);
}

void vm_sub(vm_t *vm, vm_instr_t *instr) {
    bool carry;
    bool zero;
    switch (instr->size) {
    case 0: { ARITH_OP(vm, instr, carry, zero, uint8_t, __builtin_sub_overflow); break; }
    case 1: { ARITH_OP(vm, instr, carry, zero, uint16_t, __builtin_sub_overflow); break; }
    case 2: { ARITH_OP(vm, instr, carry, zero, uint32_t, __builtin_sub_overflow); break; }
    case 3: { ARITH_OP(vm, instr, carry, zero, uint64_t, __builtin_sub_overflow); break; }
    }
    SET_FLAGS(vm, carry, zero);
}

void vm_mul(vm_t *vm, vm_instr_t *instr) {
    bool carry;
    bool zero;
    switch (instr->size) {
    case 0: { ARITH_OP(vm, instr, carry, zero, uint8_t, __builtin_mul_overflow); break; }
    case 1: { ARITH_OP(vm, instr, carry, zero, uint16_t, __builtin_mul_overflow); break; }
    case 2: { ARITH_OP(vm, instr, carry, zero, uint32_t, __builtin_mul_overflow); break; }
    case 3: { ARITH_OP(vm, instr, carry, zero, uint64_t, __builtin_mul_overflow); break; }
    }
    SET_FLAGS(vm, carry, zero);
}

#define DIV_OP(dst, src, res) \
    *((__typeof__(src)*)res) = dst / src

void vm_div(vm_t *vm, vm_instr_t *instr) {
    bool carry;
    bool zero;
    switch (instr->size) {
    case 0: { ARITH_OP(vm, instr, carry, zero, uint8_t, DIV_OP); break; }
    case 1: { ARITH_OP(vm, instr, carry, zero, uint16_t, DIV_OP); break; }
    case 2: { ARITH_OP(vm, instr, carry, zero, uint32_t, DIV_OP); break; }
    case 3: { ARITH_OP(vm, instr, carry, zero, uint64_t, DIV_OP); break; }
    }
    SET_FLAGS(vm, false, zero); // Carry might be undefined here
}

#define REM_OP(dst, src, res) \
    *((__typeof__(src)*)res) = dst % src

void vm_rem(vm_t *vm, vm_instr_t *instr) {
    bool carry;
    bool zero;
    switch (instr->size) {
    case 0: { ARITH_OP(vm, instr, carry, zero, uint8_t, REM_OP); break; }
    case 1: { ARITH_OP(vm, instr, carry, zero, uint16_t, REM_OP); break; }
    case 2: { ARITH_OP(vm, instr, carry, zero, uint32_t, REM_OP); break; }
    case 3: { ARITH_OP(vm, instr, carry, zero, uint64_t, REM_OP); break; }
    }
    SET_FLAGS(vm, false, zero); // Carry might be undefined here
}

void vm_mov(vm_t *vm, vm_instr_t *instr) {
    switch (instr->size) {
    case 0: {
        uint8_t *src = (uint8_t*)vm_src_ptr(vm, instr);
        *((uint8_t*)vm_dst_ptr(vm, instr)) = *src;
        break;
    }
    case 1: {
        uint16_t *src = (uint16_t*)vm_src_ptr(vm, instr);
        *((uint16_t*)vm_dst_ptr(vm, instr)) = *src;
        break;
    }
    case 2: {
        uint32_t *src = (uint32_t*)vm_src_ptr(vm, instr);
        *((uint32_t*)vm_dst_ptr(vm, instr)) = *src;
        break;
    }
    case 3: {
        uint64_t *src = (uint64_t*)vm_src_ptr(vm, instr);
        *((uint64_t*)vm_dst_ptr(vm, instr)) = *src;
        break;
    }
    }
    vm->cycles++; // Writing takes a cycle
}

void vm_jmp(vm_t *vm, vm_instr_t *instr) {
    // TODO: Implement relative
    uint8_t condition = vm_read8(vm, vm->registers[IP]);
    vm->registers[IP]++;
    uint64_t address = *((uint64_t*)vm_src_ptr(vm, instr));
    uint64_t flags = 0;
    if (condition & JMP_CARRY)   flags |= F_CARRY;
    if (condition & JMP_ZERO)    flags |= F_ZERO;
    if (condition & JMP_GREATER) flags |= F_GREATER;
    if (condition & JMP_LESSER)  flags |= F_LESSER;
    if (flags) {
        if (vm->registers[FLAGS] & flags) {
            vm->registers[IP] = address;
            return;
        }
        return;
    }
    vm->registers[IP] = address;
}

void vm_push(vm_t *vm, vm_instr_t *instr) {
    void *src = vm_src_ptr(vm, instr);
    switch (instr->size) {
    case 0: vm_push8(vm, *((uint8_t*)src)); break;
    case 1: vm_push16(vm, *((uint16_t*)src)); break;
    case 2: vm_push32(vm, *((uint32_t*)src)); break;
    case 3: vm_push64(vm, *((uint64_t*)src)); break;
    }
}

void vm_pop(vm_t *vm, vm_instr_t *instr) {
    void *dst = vm_dst_ptr(vm, instr);
    switch (instr->size) {
    case 0: *((uint8_t*)dst) = vm_pop8(vm); break;
    case 1: *((uint16_t*)dst) = vm_pop16(vm); break;
    case 2: *((uint32_t*)dst) = vm_pop32(vm); break;
    case 3: *((uint64_t*)dst) = vm_pop64(vm); break;
    }
    vm->cycles++; // Writing takes a cycle
}

// TODO: Handle relative
void vm_call(vm_t *vm, vm_instr_t *instr) {
    uint64_t address = *((uint64_t*)vm_src_ptr(vm, instr));
    vm_push64(vm, vm->registers[IP]);
    vm->registers[IP] = address;
}

void vm_ret(vm_t *vm, vm_instr_t *instr) {
    vm->registers[IP] = vm_pop64(vm);
}

void vm_and(vm_t *vm, vm_instr_t *instr) {
    void *src = vm_src_ptr(vm, instr);
    void *dst = vm_dst_ptr(vm, instr);
    switch (instr->size) {
    case 0: *((uint8_t*)dst) = *((uint8_t*)dst) & *((uint8_t*)src); break;
    case 1: *((uint16_t*)dst) = *((uint16_t*)dst) & *((uint16_t*)src); break;
    case 2: *((uint32_t*)dst) = *((uint32_t*)dst) & *((uint32_t*)src); break;
    case 3: *((uint64_t*)dst) = *((uint64_t*)dst) & *((uint64_t*)src); break;
    }
    vm->cycles++; // Writing takes a cycle
}

void vm_or(vm_t *vm, vm_instr_t *instr) {
    void *src = vm_src_ptr(vm, instr);
    void *dst = vm_dst_ptr(vm, instr);
    switch (instr->size) {
    case 0: *((uint8_t*)dst) = *((uint8_t*)dst) | *((uint8_t*)src); break;
    case 1: *((uint16_t*)dst) = *((uint16_t*)dst) | *((uint16_t*)src); break;
    case 2: *((uint32_t*)dst) = *((uint32_t*)dst) | *((uint32_t*)src); break;
    case 3: *((uint64_t*)dst) = *((uint64_t*)dst) | *((uint64_t*)src); break;
    }
    vm->cycles++; // Writing takes a cycle
}

void vm_xor(vm_t *vm, vm_instr_t *instr) {
    void *src = vm_src_ptr(vm, instr);
    void *dst = vm_dst_ptr(vm, instr);
    switch (instr->size) {
    case 0: *((uint8_t*)dst) = *((uint8_t*)dst) ^ *((uint8_t*)src); break;
    case 1: *((uint16_t*)dst) = *((uint16_t*)dst) ^ *((uint16_t*)src); break;
    case 2: *((uint32_t*)dst) = *((uint32_t*)dst) ^ *((uint32_t*)src); break;
    case 3: *((uint64_t*)dst) = *((uint64_t*)dst) ^ *((uint64_t*)src); break;
    }
    vm->cycles++; // Writing takes a cycle
}

void vm_not(vm_t *vm, vm_instr_t *instr) {
    void *dst = vm_dst_ptr(vm, instr);
    switch (instr->size) {
    case 0: *((uint8_t*)dst) = ~*((uint8_t*)dst); break;
    case 1: *((uint16_t*)dst) = ~*((uint16_t*)dst); break;
    case 2: *((uint32_t*)dst) = ~*((uint32_t*)dst); break;
    case 3: *((uint64_t*)dst) = ~*((uint64_t*)dst); break;
    }
    vm->cycles++; // Writing takes a cycle
}

void vm_shl(vm_t *vm, vm_instr_t *instr) {
    void *src = vm_src_ptr(vm, instr);
    void *dst = vm_dst_ptr(vm, instr);
    switch (instr->size) {
    case 0: *((uint8_t*)dst) = *((uint8_t*)dst) << *((uint8_t*)src); break;
    case 1: *((uint16_t*)dst) = *((uint16_t*)dst) << *((uint16_t*)src); break;
    case 2: *((uint32_t*)dst) = *((uint32_t*)dst) << *((uint32_t*)src); break;
    case 3: *((uint64_t*)dst) = *((uint64_t*)dst) << *((uint64_t*)src); break;
    }
    vm->cycles++; // Writing takes a cycle
}

void vm_shr(vm_t *vm, vm_instr_t *instr) {
    void *src = vm_src_ptr(vm, instr);
    void *dst = vm_dst_ptr(vm, instr);
    switch (instr->size) {
    case 0: *((uint8_t*)dst) = *((uint8_t*)dst) >> *((uint8_t*)src); break;
    case 1: *((uint16_t*)dst) = *((uint16_t*)dst) >> *((uint16_t*)src); break;
    case 2: *((uint32_t*)dst) = *((uint32_t*)dst) >> *((uint32_t*)src); break;
    case 3: *((uint64_t*)dst) = *((uint64_t*)dst) >> *((uint64_t*)src); break;
    }
    vm->cycles++; // Writing takes a cycle
}

void vm_sei(vm_t *vm, vm_instr_t *instr) {
    vm->registers[FLAGS] |= F_INT;
    vm->cycles++;
}

void vm_sdi(vm_t *vm, vm_instr_t *instr) {
    vm->registers[FLAGS] &= ~F_INT;
    vm->cycles++;
}

void vm_int(vm_t *vm, vm_instr_t *instr) {
    printf("INT Not implemented.\n");
    vm->halted = true;
}

void vm_cmp(vm_t *vm, vm_instr_t *instr) {
    vm->registers[FLAGS] &= ~(F_GREATER | F_LESSER | F_ZERO);
    uint64_t a = 0;
    uint64_t b = 0;
    switch (instr->size) {
    case 0:
        b = *((uint8_t*)vm_src_ptr(vm, instr));
        a = *((uint8_t*)vm_dst_ptr(vm, instr));
        break;
    case 1:
        b = *((uint16_t*)vm_src_ptr(vm, instr));
        a = *((uint16_t*)vm_dst_ptr(vm, instr));
        break;
    case 2:
        b = *((uint32_t*)vm_src_ptr(vm, instr));
        a = *((uint32_t*)vm_dst_ptr(vm, instr));
        break;
    case 3:
        b = *((uint64_t*)vm_src_ptr(vm, instr));
        a = *((uint64_t*)vm_dst_ptr(vm, instr));
        break;
    }
    if (a < b) vm->registers[FLAGS] |= F_LESSER;
    else if (a > b) vm->registers[FLAGS] |= F_GREATER;
    if (a == b) vm->registers[FLAGS] |= F_ZERO;
}

void vm_dump(vm_t *vm) {
    for (int i = 0; i <= 10; i++)
        printf("G%d = 0x%lx\n", i, vm->registers[i]);
    printf("SP = 0x%lx\n", vm->registers[SP]);
    printf("FP = 0x%lx\n", vm->registers[FP]);
    printf("IP = 0x%lx\n", vm->registers[IP]);
    printf("FLAGS = 0x%lx\n", vm->registers[FLAGS]);
    printf("PGTBL = 0x%lx\n", vm->registers[PGTBL]);
    printf("IVTBL = 0x%lx\n", vm->registers[IVTBL]);
}

void vm_destroy(vm_t *vm) {
    free(vm->ram);
    free(vm);
}

op_func vm_op_lookup[64] = {
    vm_nop, vm_add, vm_sub, vm_mul, vm_div, vm_mov, vm_jmp,
    vm_push, vm_pop, vm_call, vm_ret, vm_and, vm_or, vm_xor,
    vm_not, vm_shl, vm_shr, vm_sei, vm_sdi, vm_int, vm_cmp,
    vm_rem
};
