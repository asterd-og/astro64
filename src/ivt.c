#include "ivt.h"
#include "mem.h"
#include <stdio.h>

void vm_raise(vm_t *vm, uint8_t vector) {
    if (!(vm->registers[FLAGS] & F_INT))
        return;
    vm_ivt_t *tbl = (vm_ivt_t*)vm_ptr(vm, vm->registers[IVTBL]);
    if (vector > tbl->count - 1)
        return;
    vm_int_t *entry = (vm_int_t*)vm_ptr(vm, (uint64_t)(tbl->table + vector));
    if (!(entry->flags & 1))
        return; // Interrupt not present
    vm->queued_int = entry;
}

void vm_run_queued_int(vm_t *vm) {
    if (!vm->queued_int)
        return;
    vm_push64(vm, vm->registers[IP]);
    vm->registers[IP] = vm->queued_int->handler_addr;
    vm->queued_int = NULL;
}
