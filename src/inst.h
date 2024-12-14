#pragma once

#include <stdint.h>
#include <stddef.h>
#include "vm.h"

#define INST_MOV 0b0000000

#define INST_FLAGS_OFF_SRC 0b0001
#define INST_FLAGS_OFF_DST 0b0010
#define INST_FLAGS_REG_OFF_SRC 0b0100
#define INST_FLAGS_REG_OFF_DST 0b1000

#define INST_JMP_CARRY 0b00000010
#define INST_JMP_ZERO 0b00000100

typedef struct {
    uint8_t Size : 2;
    uint8_t OpCode : 6;
    uint8_t Src: 2;
    uint8_t Dst: 2;
    uint8_t Flags : 4;
} __attribute__((packed)) AstroVmInst;

typedef void(*AstroVmInstHandler)(AstroVm*, AstroVmInst*);

AstroVmInst AstroVmGetInst(AstroVm *Vm);
uint8_t AstroVmGetWordSize(uint8_t Word);

