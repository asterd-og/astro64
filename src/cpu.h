#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "instr.h"

#define SP 11
#define FP 12
#define IP 13
#define FLAGS 14
#define PGTBL 15
#define IVTBL 16

#define F_CARRY 1
#define F_ZERO 2
#define F_PAGING 4
#define F_INT 8
#define F_GREATER 16
#define F_LESSER 32

#define SZ(x) 1 << x

typedef struct {
    uint64_t handler_addr;
    uint8_t flags;
} __attribute__((packed)) vm_int_t;

typedef struct {
    vm_int_t *table;
    uint8_t count;
} __attribute__((packed)) vm_ivt_t;

typedef struct {
    uint64_t registers[17];
    bool halted;
    uint8_t *ram;
    size_t ram_size;
    vm_int_t *queued_int;
} vm_t;

typedef void(*op_func)(vm_t*, vm_instr_t*);

extern op_func vm_op_lookup[64];

vm_t *vm_create(size_t ram_size);
void vm_load_rom(vm_t *vm, uint8_t *rom, size_t rom_size);
void vm_clock(vm_t *vm); // Returns cycles ran
void vm_push8(vm_t *vm, uint8_t data);
void vm_push16(vm_t *vm, uint16_t data);
void vm_push32(vm_t *vm, uint32_t data);
void vm_push64(vm_t *vm, uint64_t data);
void vm_dump(vm_t *vm);
void vm_destroy(vm_t *vm);
