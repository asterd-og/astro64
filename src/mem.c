#include "mem.h"
#include "int.h"
#include <stdio.h>

// Every page map level NEEDS to be page aligned.

uint64_t AstroVmTraverseExpect(AstroVm *Vm, uint64_t *Level, uint8_t Entry, uint8_t Flags) {
    if ((Level[Entry] & Flags) != Flags) {
        AstroVmRaiseInt(Vm, PAGE_FAULT_INT);
        return 1; // It should never return 1 if it finds a valid entry because it masks out 0xff
    }
    return (uint64_t)(Level[Entry] & ~(uint8_t)0xFF);
}

uint64_t AstroVmGetPhysPage(AstroVm *Vm, uint64_t Address, uint8_t Flags) {
    uint8_t Pml5Idx = (Address >> 43) & 0xff;
    uint8_t Pml4Idx = (Address >> 35) & 0xff;
    uint8_t Pml3Idx = (Address >> 27) & 0xff;
    uint8_t Pml2Idx = (Address >> 19) & 0xff;
    uint8_t Pml1Idx = (Address >> 11) & 0xff;

    uint64_t Pml5 = AstroVmTraverseExpect(Vm, (uint64_t*)(Vm->Ram + Vm->Registers[REG_PGTBL]), Pml5Idx, Flags);
    if (Pml5 == 1) return 0;
    uint64_t Pml4 = AstroVmTraverseExpect(Vm, (uint64_t*)(Vm->Ram + Pml5), Pml4Idx, Flags);
    if (Pml4 == 1) return 0;
    uint64_t Pml3 = AstroVmTraverseExpect(Vm, (uint64_t*)(Vm->Ram + Pml4), Pml3Idx, Flags);
    if (Pml3 == 1) return 0;
    uint64_t Pml2 = AstroVmTraverseExpect(Vm, (uint64_t*)(Vm->Ram + Pml3), Pml2Idx, Flags);
    if (Pml2 == 1) return 0;
    uint64_t Pml1 = AstroVmTraverseExpect(Vm, (uint64_t*)(Vm->Ram + Pml2), Pml1Idx, Flags);
    if (Pml1 == 1) return 0;
    return Pml1;
}

uint64_t AstroVmRead64(AstroVm *Vm, uint64_t Address) {
    return *(uint64_t*)AstroVmGetPtr(Vm, Address);
}

uint32_t AstroVmRead32(AstroVm *Vm, uint64_t Address) {
    return *(uint32_t*)AstroVmGetPtr(Vm, Address);
}

uint16_t AstroVmRead16(AstroVm *Vm, uint64_t Address) {
    return *(uint16_t*)AstroVmGetPtr(Vm, Address);
}

uint8_t AstroVmRead8(AstroVm *Vm, uint64_t Address) {
    return *(uint8_t*)AstroVmGetPtr(Vm, Address);
}

void AstroVmWrite64(AstroVm *Vm, uint64_t Address, uint64_t Data) {
    *(uint64_t*)AstroVmGetPtr(Vm, Address) = Data;
}

void AstroVmWrite32(AstroVm *Vm, uint64_t Address, uint32_t Data) {
    *(uint32_t*)AstroVmGetPtr(Vm, Address) = Data;
}

void AstroVmWrite16(AstroVm *Vm, uint64_t Address, uint16_t Data) {
    *(uint16_t*)AstroVmGetPtr(Vm, Address) = Data;
}

void AstroVmWrite8(AstroVm *Vm, uint64_t Address, uint8_t Data) {
    *(uint8_t*)AstroVmGetPtr(Vm, Address) = Data;
}

uint64_t AstroVmRead(AstroVm *Vm, uint64_t Address, uint8_t Size) {
    switch (Size) {
    case 0:
        return AstroVmRead8(Vm, Address);
    case 1:
        return AstroVmRead16(Vm, Address);
    case 2:
        return AstroVmRead32(Vm, Address);
    case 3:
        return AstroVmRead64(Vm, Address);
    }
    // TODO: Error
    return 0;
}

void AstroVmWrite(AstroVm *Vm, uint64_t Address, uint64_t Data, uint8_t Size) {
    switch (Size) {
    case 0:
        AstroVmWrite8(Vm, Address, Data);
        break;
    case 1:
        AstroVmWrite16(Vm, Address, Data);
        break;
    case 2:
        AstroVmWrite32(Vm, Address, Data);
        break;
    case 3:
        AstroVmWrite64(Vm, Address, Data);
        break;
    }
}

void *AstroVmGetPtr(AstroVm *Vm, uint64_t Address) {
    if (Vm->Registers[REG_FLAGS] & REG_FLAGS_PAGING) {
        uint64_t PhysPage = AstroVmGetPhysPage(Vm, Address, MM_READ);
        uint64_t Offset = Address & 0x7ff;
        void *AccessPtr = (void*)(Vm->Ram + PhysPage + Offset);
        return AccessPtr;
    }
    if (Address > Vm->RamSize) {
        AstroVmRaiseInt(Vm, PAGE_FAULT_INT);
        return (void*)(Vm->Ram);
    }
    return (void*)(Vm->Ram + Address);
}
