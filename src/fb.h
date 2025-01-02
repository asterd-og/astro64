#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "vm.h"

extern bool DeviceFbFinished;

void DeviceFbRegister(AstroVm *Vm, uint32_t Width, uint32_t Height);
void DeviceFbRun();
void DeviceFbUpdate(AstroVm *Vm);
