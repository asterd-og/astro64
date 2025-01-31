#pragma once

#include <stdint.h>
#include <stddef.h>
#include "cpu.h"

void vm_init_devices(vm_t *vm);
void vm_update_devices(vm_t *vm);