#include "mem.h"

// Every page map level NEEDS to be page aligned.

uint64_t AstroVmTraverseExpect(uint64_t *Level, uint8_t Entry, uint8_t Flags) {
    if ((Level[Entry] & Flags) != Flags) {
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

    uint64_t Pml5 = AstroVmTraverseExpect((uint64_t*)(Vm->Ram + Vm->Registers[REG_PGTBL]), Pml5Idx, Flags);
    uint64_t Pml4 = AstroVmTraverseExpect((uint64_t*)(Vm->Ram + Pml5), Pml4Idx, Flags);
    uint64_t Pml3 = AstroVmTraverseExpect((uint64_t*)(Vm->Ram + Pml4), Pml3Idx, Flags);
    uint64_t Pml2 = AstroVmTraverseExpect((uint64_t*)(Vm->Ram + Pml3), Pml2Idx, Flags);
    uint64_t Pml1 = AstroVmTraverseExpect((uint64_t*)(Vm->Ram + Pml2), Pml1Idx, Flags);
    return Pml1;
}

#define READ_PHYS(Vm, Address, Type)      \
    if (Address >= Vm->RamSize) { \
    Vm->Registers[REG_ERR] |= REG_ERR_PAGE_FAULT; \
    return 0; \
    } \
    Type *AccessPtr = (Type*)(Vm->Ram + Address); \
    return *AccessPtr

uint64_t AstroVmRead64(AstroVm *Vm, uint64_t Address) {
    if (Vm->Registers[REG_FLAGS] & REG_FLAGS_PAGING) {
        uint64_t PhysPage = AstroVmGetPhysPage(Vm, Address, MM_READ);
        uint64_t Offset = Address & 0x7ff;
        uint64_t *AccessPtr = (uint64_t*)(Vm->Ram + PhysPage + Offset);
        return *AccessPtr;
    }
    READ_PHYS(Vm, Address, uint64_t);
}

uint32_t AstroVmRead32(AstroVm *Vm, uint64_t Address) {
    if (Vm->Registers[REG_FLAGS] & REG_FLAGS_PAGING) {
        uint64_t PhysPage = AstroVmGetPhysPage(Vm, Address, MM_READ);
        uint64_t Offset = Address & 0x7ff;
        uint32_t *AccessPtr = (uint32_t*)(Vm->Ram + PhysPage + Offset);
        return *AccessPtr;
    }
    READ_PHYS(Vm, Address, uint32_t);
}

uint16_t AstroVmRead16(AstroVm *Vm, uint64_t Address) {
    if (Vm->Registers[REG_FLAGS] & REG_FLAGS_PAGING) {
        uint64_t PhysPage = AstroVmGetPhysPage(Vm, Address, MM_READ);
        uint64_t Offset = Address & 0x7ff;
        uint16_t *AccessPtr = (uint16_t*)(Vm->Ram + PhysPage + Offset);
        return *AccessPtr;
    }
    READ_PHYS(Vm, Address, uint16_t);
}

uint8_t AstroVmRead8(AstroVm *Vm, uint64_t Address) {
    if (Vm->Registers[REG_FLAGS] & REG_FLAGS_PAGING) {
        uint64_t PhysPage = AstroVmGetPhysPage(Vm, Address, MM_READ);
        uint64_t Offset = Address & 0x7ff;
        uint8_t *AccessPtr = (uint8_t*)(Vm->Ram + PhysPage + Offset);
        return *AccessPtr;
    }
    READ_PHYS(Vm, Address, uint8_t);
}

#define WRITE_PHYS(Vm, Address, Data, Type)      \
    if (Address >= Vm->RamSize) { \
    Vm->Registers[REG_ERR] |= REG_ERR_PAGE_FAULT; \
    return; \
    } \
    Type *AccessPtr = (Type*)(Vm->Ram + Address); \
    *AccessPtr = Data

void AstroVmWrite64(AstroVm *Vm, uint64_t Address, uint64_t Data) {
    if (Vm->Registers[REG_FLAGS] & REG_FLAGS_PAGING) {
        uint64_t PhysPage = AstroVmGetPhysPage(Vm, Address, MM_READ | MM_WRITE);
        uint64_t Offset = Address & 0x7ff;
        uint64_t *AccessPtr = (uint64_t*)(Vm->Ram + PhysPage + Offset);
        *AccessPtr = Data;
        return;
    }
    WRITE_PHYS(Vm, Address, Data, uint64_t);
}

void AstroVmWrite32(AstroVm *Vm, uint64_t Address, uint32_t Data) {
    if (Vm->Registers[REG_FLAGS] & REG_FLAGS_PAGING) {
        uint64_t PhysPage = AstroVmGetPhysPage(Vm, Address, MM_READ | MM_WRITE);
        uint64_t Offset = Address & 0x7ff;
        uint32_t *AccessPtr = (uint32_t*)(Vm->Ram + PhysPage + Offset);
        *AccessPtr = Data;
        return;
    }
    WRITE_PHYS(Vm, Address, Data, uint32_t);
}

void AstroVmWrite16(AstroVm *Vm, uint64_t Address, uint16_t Data) {
    if (Vm->Registers[REG_FLAGS] & REG_FLAGS_PAGING) {
        uint64_t PhysPage = AstroVmGetPhysPage(Vm, Address, MM_READ | MM_WRITE);
        uint64_t Offset = Address & 0x7ff;
        uint16_t *AccessPtr = (uint16_t*)(Vm->Ram + PhysPage + Offset);
        *AccessPtr = Data;
        return;
    }
    WRITE_PHYS(Vm, Address, Data, uint16_t);
}

void AstroVmWrite8(AstroVm *Vm, uint64_t Address, uint8_t Data) {
    if (Vm->Registers[REG_FLAGS] & REG_FLAGS_PAGING) {
        uint64_t PhysPage = AstroVmGetPhysPage(Vm, Address, MM_READ | MM_WRITE);
        uint64_t Offset = Address & 0x7ff;
        uint8_t *AccessPtr = (uint8_t*)(Vm->Ram + PhysPage + Offset);
        *AccessPtr = Data;
        return;
    }
    WRITE_PHYS(Vm, Address, Data, uint8_t);
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
