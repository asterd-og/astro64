#pragma once

#include <stdint.h>
#include <stddef.h>
#include "cpu.h"

#define I_PF 1
#define I_VBLANK 2

void vm_raise(vm_t *vm, uint8_t vector);
void vm_run_queued_int(vm_t *vm);
