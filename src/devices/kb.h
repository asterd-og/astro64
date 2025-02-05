#pragma once

#include <stdint.h>
#include <stddef.h>
#include "../cpu.h"

enum {
    KB_KEY = 0x12000,
    KB_STATUS = 0x12004
};

#define KB_INT 10

void vm_kb_init(vm_t *vm);
void vm_kb_unload();