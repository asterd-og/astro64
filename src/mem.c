#include "mem.h"

void *vm_ptr(vm_t *vm, uint64_t address) {
    if (vm->registers[FLAGS] & F_PAGING) {
        // TODO.
        return NULL;
    }
    return (void*)(vm->ram + address);
}

uint8_t vm_read8(vm_t *vm, uint64_t address) {
    return *((uint8_t*)vm_ptr(vm, address));
}

uint16_t vm_read16(vm_t *vm, uint64_t address) {
    return *((uint16_t*)vm_ptr(vm, address));
}

uint32_t vm_read32(vm_t *vm, uint64_t address) {
    return *((uint32_t*)vm_ptr(vm, address));
}

uint64_t vm_read64(vm_t *vm, uint64_t address) {
    return *((uint64_t*)vm_ptr(vm, address));
}

void vm_write8(vm_t *vm, uint64_t address, uint8_t data) {
    *((uint8_t*)vm_ptr(vm, address)) = data;
}

void vm_write16(vm_t *vm, uint64_t address, uint16_t data) {
    *((uint16_t*)vm_ptr(vm, address)) = data;
}

void vm_write32(vm_t *vm, uint64_t address, uint32_t data) {
    *((uint32_t*)vm_ptr(vm, address)) = data;
}

void vm_write64(vm_t *vm, uint64_t address, uint64_t data) {
    *((uint64_t*)vm_ptr(vm, address)) = data;
}

uint64_t vm_read(vm_t *vm, uint64_t address, uint8_t size) {
    switch (size) {
    case 0: return vm_read8(vm, address);
    case 1: return vm_read16(vm, address);
    case 2: return vm_read32(vm, address);
    case 3: return vm_read64(vm, address);
    }
    return 0;
}

void vm_write(vm_t *vm, uint64_t address, uint64_t data, uint8_t size) {
    switch (size) {
    case 0: vm_write8(vm, address, data); break;
    case 1: vm_write16(vm, address, data); break;
    case 2: vm_write32(vm, address, data); break;
    case 3: vm_write64(vm, address, data); break;
    }
}
