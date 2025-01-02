#pragma once

#include <stdint.h>
#include <stddef.h>
#include "vm.h"

#define PAGE_FAULT_INT 1

void AstroVmRaiseInt(AstroVm *Vm, uint8_t Vector);
void AstroVmRunQueuedInt(AstroVm *Vm);
