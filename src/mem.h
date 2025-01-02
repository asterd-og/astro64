#pragma once

#include <stdint.h>
#include <stddef.h>
#include "vm.h"

#define MM_READ 1
#define MM_WRITE 2
#define MM_NOEXEC 4
#define MM_USER 8

uint64_t AstroVmRead64(AstroVm *Vm, uint64_t Address);
uint32_t AstroVmRead32(AstroVm *Vm, uint64_t Address);
uint16_t AstroVmRead16(AstroVm *Vm, uint64_t Address);
uint8_t AstroVmRead8(AstroVm *Vm, uint64_t Address);

void AstroVmWrite64(AstroVm *Vm, uint64_t Address, uint64_t Data);
void AstroVmWrite32(AstroVm *Vm, uint64_t Address, uint32_t Data);
void AstroVmWrite16(AstroVm *Vm, uint64_t Address, uint16_t Data);
void AstroVmWrite8(AstroVm *Vm, uint64_t Address, uint8_t Data);

uint64_t AstroVmRead(AstroVm *Vm, uint64_t Address, uint8_t Size);
void AstroVmWrite(AstroVm *Vm, uint64_t Address, uint64_t Data, uint8_t Size);

void *AstroVmGetPtr(AstroVm *Vm, uint64_t Address);

