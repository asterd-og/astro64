#pragma once

#include <stdint.h>
#include <stddef.h>
#include "vm.h"

typedef struct {
    uint64_t Id;
    uint64_t Address;
} __attribute__((packed)) AstroVmIoDevice;

void AstroVmRegisterDevice(AstroVm *Vm, uint64_t Id, uint64_t Address);

