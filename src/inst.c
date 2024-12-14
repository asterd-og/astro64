#include "inst.h"
#include "mem.h"

AstroVmInst AstroVmGetInst(AstroVm *Vm) {
    // TODO: Make it read from mem
    uint16_t Instruction = AstroVmRead16(Vm, Vm->Registers[REG_IP]);
    AstroVmInst Inst = {
        .Size = (Instruction >> 14) & 0b11,
        .OpCode = (Instruction >> 8) & 0b111111,
        .Src = (Instruction >> 6) & 0b11,
        .Dst = (Instruction >> 4) & 0b11,
        .Flags = Instruction & 0b1111
    };
    return Inst;
}

uint8_t AstroVmGetWordSize(uint8_t Word) {
    switch (Word) {
    case 0:
        return 1;
    case 1:
        return 2;
    case 2:
        return 4;
    case 3:
        return 8;
    }
}

