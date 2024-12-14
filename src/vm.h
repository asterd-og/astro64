#pragma once

#include <stdint.h>
#include <stddef.h>

#define REG_G0 0
#define REG_G1 1
#define REG_G2 2
#define REG_G3 3
#define REG_G4 4
#define REG_G5 5
#define REG_G6 6
#define REG_G7 7
#define REG_G8 8
#define REG_G9 9
#define REG_G10 10
#define REG_SP 11
#define REG_IP 12
#define REG_FLAGS 13
#define REG_PGTBL 14
#define REG_ERR 15

#define REG_FLAGS_CARRY 1
#define REG_FLAGS_ZERO 2
#define REG_FLAGS_PAGING 4

#define REG_ERR_PAGE_FAULT 4

typedef struct {
    // Registers
    uint64_t Registers[16];

    uint8_t *Ram;
    size_t RamSize;
} AstroVm;

AstroVm *AstroVmInitialise(size_t RamSize);
void AstroVmLoadProgram(AstroVm *Vm, uint8_t *Data, size_t Size);
void AstroVmStep(AstroVm *Vm);
void AstroVmDumpRegs(AstroVm *Vm);
void AstroVmDestroy(AstroVm *Vm);
