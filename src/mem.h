#pragma once

#include <stdint.h>
#include <stddef.h>
#include "cpu.h"

void *vm_ptr(vm_t *vm, uint64_t address);

uint8_t vm_read8(vm_t *vm, uint64_t address);
uint16_t vm_read16(vm_t *vm, uint64_t address);
uint32_t vm_read32(vm_t *vm, uint64_t address);
uint64_t vm_read64(vm_t *vm, uint64_t address);

void vm_write8(vm_t *vm, uint64_t address, uint8_t data);
void vm_write16(vm_t *vm, uint64_t address, uint16_t data);
void vm_write32(vm_t *vm, uint64_t address, uint32_t data);
void vm_write64(vm_t *vm, uint64_t address, uint64_t data);

uint64_t vm_read(vm_t *vm, uint64_t address, uint8_t size);
void vm_write(vm_t *vm, uint64_t address, uint64_t data, uint8_t size);

