#include "vm.h"
#include "inst.h"
#include "mem.h"
#include "int.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void AstroVmNopInst(AstroVm*, AstroVmInst*);
void AstroVmAddInst(AstroVm*, AstroVmInst*);
void AstroVmSubInst(AstroVm*, AstroVmInst*);
void AstroVmMulInst(AstroVm*, AstroVmInst*);
void AstroVmDivInst(AstroVm*, AstroVmInst*);
void AstroVmMovInst(AstroVm*, AstroVmInst*);
void AstroVmJmpInst(AstroVm*, AstroVmInst*);
void AstroVmPushInst(AstroVm*, AstroVmInst*);
void AstroVmPopInst(AstroVm*, AstroVmInst*);
void AstroVmCallInst(AstroVm*, AstroVmInst*);
void AstroVmRetInst(AstroVm*, AstroVmInst*);
void AstroVmAndInst(AstroVm*, AstroVmInst*);
void AstroVmOrInst(AstroVm*, AstroVmInst*);
void AstroVmXorInst(AstroVm*, AstroVmInst*);
void AstroVmNotInst(AstroVm*, AstroVmInst*);
void AstroVmShlInst(AstroVm*, AstroVmInst*);
void AstroVmShrInst(AstroVm*, AstroVmInst*);
void AstroVmSeiInst(AstroVm*, AstroVmInst*);
void AstroVmSdiInst(AstroVm*, AstroVmInst*);
void AstroVmIntInst(AstroVm*, AstroVmInst*);

AstroVmInstHandler AstroVmInstHandlers[64] = {
    AstroVmNopInst, AstroVmAddInst, AstroVmSubInst,
    AstroVmMulInst, AstroVmDivInst, AstroVmMovInst,
    AstroVmJmpInst, AstroVmPushInst, AstroVmPopInst,
    AstroVmCallInst, AstroVmRetInst, AstroVmAndInst,
    AstroVmOrInst, AstroVmXorInst, AstroVmNotInst,
    AstroVmShlInst, AstroVmShrInst, AstroVmSeiInst,
    AstroVmSdiInst, AstroVmIntInst
};

AstroVm *AstroVmInitialise(size_t RamSize) {
    AstroVm *Vm = (AstroVm*)malloc(sizeof(AstroVm));
    memset(Vm, 0, sizeof(AstroVm));
    Vm->Ram = (uint8_t*)malloc(RamSize);
    if (!Vm->Ram) {
        printf("Couldn't allocate enough space for RAM.\n");
        return NULL;
    }
    Vm->RamSize = RamSize;
    return Vm;
}

void AstroVmLoadProgram(AstroVm *Vm, uint8_t *Data, size_t Size) {
    if (Size > Vm->RamSize) {
        printf("Failed loading program! Too big.\n");
        return; // TODO: Error VM.
    }
    memcpy(Vm->Ram, Data, Size);
}

void AstroVmPush(AstroVm *Vm, uint64_t Data, size_t Size) {
    Vm->Registers[REG_SP] -= 1 << Size;
    AstroVmWrite(Vm, Vm->Registers[REG_SP], Data, Size);
}

uint64_t AstroVmPop(AstroVm *Vm, size_t Size) {
    uint64_t Data = AstroVmRead(Vm, Vm->Registers[REG_SP], Size);
    Vm->Registers[REG_SP] += 1 << Size;
    return Data;
}

void AstroVmStep(AstroVm *Vm, bool Debug) {
    AstroVmInst Inst = AstroVmGetInst(Vm);
    if (Debug)
        printf("Size: %d, Op code: %d, Flags: %d\n",
               Inst.Size, Inst.OpCode, Inst.Flags);
    Vm->Registers[REG_IP] += 2;
    AstroVmInstHandlers[Inst.OpCode](Vm, &Inst);
    AstroVmRunQueuedInt(Vm);
}

uint64_t AstroVmGetSrc(AstroVm *Vm, AstroVmInst *Inst, bool IncIP) {
    uint64_t Src = 0;
    uint64_t SrcOff = 0;
    uint64_t IP = Vm->Registers[REG_IP];

    // Src Offset
    if (Inst->Flags & INST_FLAGS_OFF_SRC) {
        if (Inst->Flags & INST_FLAGS_REG_OFF_SRC) {
            SrcOff = Vm->Registers[AstroVmRead8(Vm, IP)];
            IP++;
        } else {
            SrcOff = AstroVmRead64(Vm, IP);
            IP += 8;
        }
    }

    // Source
    uint64_t Address = 0;
    switch (Inst->Src) {
    case 0:
        // Register
        Src = Vm->Registers[AstroVmRead8(Vm, IP)];
        IP++;
        break;
    case 1:
        // [Register]
        Address = Vm->Registers[AstroVmRead8(Vm, IP)];
        Address += SrcOff;
        Src = AstroVmRead(Vm, Address, Inst->Size);
        IP++;
        break;
    case 2:
        // IMM
        Src = AstroVmRead(Vm, IP, Inst->Size);
        IP += 1 << Inst->Size;
        break;
    case 3:
        // [IMM]
        Address = AstroVmRead64(Vm, IP);
        Address += SrcOff;
        Src = AstroVmRead(Vm, Address, Inst->Size);
        IP += 8;
        break;
    }

    if (IncIP)
        Vm->Registers[REG_IP] = IP;

    return Src;
}

void AstroVmSetDst(AstroVm *Vm, AstroVmInst *Inst, uint64_t Src, bool IncIP) {
    uint64_t DstOff = 0;
    uint64_t IP = Vm->Registers[REG_IP];

    // Dst Offset
    if (Inst->Flags & INST_FLAGS_OFF_DST) {
        if (Inst->Flags & INST_FLAGS_REG_OFF_DST) {
            DstOff = Vm->Registers[AstroVmRead8(Vm, IP)];
            IP++;
        } else {
            DstOff = AstroVmRead64(Vm, IP);
            IP += 8;
        }
    }

    // Destination
    uint64_t Address = 0;
    switch (Inst->Dst) {
    case 0:
        // Register
        Vm->Registers[AstroVmRead8(Vm, IP)] = Src;
        IP++;
        break;
    case 1:
        // [Register]
        Address = Vm->Registers[AstroVmRead8(Vm, IP)];
        Address += DstOff;
        AstroVmWrite(Vm, Address, Src, Inst->Size);
        IP++;
        break;
    case 2:
        // [Imm]
        Address = AstroVmRead64(Vm, IP);
        Address += DstOff;
        AstroVmWrite(Vm, Address, Src, Inst->Size);
        IP += 8;
        break;
    }

    if (IncIP)
        Vm->Registers[REG_IP] = IP;
}

uint64_t AstroVmGetDst(AstroVm *Vm, AstroVmInst *Inst, bool IncIP) {
    uint64_t Dst = 0;
    uint64_t DstOff = 0;
    uint64_t IP = Vm->Registers[REG_IP];

    // Dst Offset
    if (Inst->Flags & INST_FLAGS_OFF_DST) {
        if (Inst->Flags & INST_FLAGS_REG_OFF_DST) {
            DstOff = Vm->Registers[AstroVmRead8(Vm, IP)];
            IP++;
        } else {
            DstOff = AstroVmRead64(Vm, IP);
            IP += 8;
        }
    }

    // Source
    uint64_t Address = 0;
    switch (Inst->Dst) {
    case 0:
        // Register
        Dst = Vm->Registers[AstroVmRead8(Vm, IP)];
        IP++;
        break;
    case 1:
        // [Register]
        Address = Vm->Registers[AstroVmRead8(Vm, IP)];
        Address += DstOff;
        Dst = AstroVmRead(Vm, Address, Inst->Size);
        IP++;
        break;
    case 2:
        // [IMM]
        Address = AstroVmRead64(Vm, IP);
        Address += DstOff;
        Dst = AstroVmRead(Vm, Address, Inst->Size);
        IP += 8;
        break;
    }

    if (IncIP)
        Vm->Registers[REG_IP] = IP;

    return Dst;
}

void AstroVmNopInst(AstroVm *Vm, AstroVmInst *Inst) {
    printf("Hit NOP!\n");
    // TODO
}

#define OpCheckCarry(Vm, Inst, Carry, Zero, Type, Op)   \
    Type Src = (Type)AstroVmGetSrc(Vm, Inst, true); \
    Type Dst = (Type)AstroVmGetDst(Vm, Inst, false); \
    Type Res = 0; \
    Carry = Op(Dst, Src, &Res); \
    AstroVmSetDst(Vm, Inst, Res, true); \
    Zero = Res == 0

#define SET_FLAGS(Vm, Carry, Zero) \
    if (Carry) \
        Vm->Registers[REG_FLAGS] |= REG_FLAGS_CARRY; \
    else \
        Vm->Registers[REG_FLAGS] &= ~REG_FLAGS_CARRY; \
    if (Zero) \
        Vm->Registers[REG_FLAGS] |= REG_FLAGS_ZERO; \
    else \
        Vm->Registers[REG_FLAGS] &= ~REG_FLAGS_ZERO

void AstroVmAddInst(AstroVm *Vm, AstroVmInst *Inst) {
    bool Carry = false;
    bool Zero = false;
    switch (Inst->Size) {
    case 0: {
        OpCheckCarry(Vm, Inst, Carry, Zero, uint8_t, __builtin_add_overflow);
        break;
    }
    case 1: {
        OpCheckCarry(Vm, Inst, Carry, Zero, uint16_t, __builtin_add_overflow);
        break;
    }
    case 2: {
        OpCheckCarry(Vm, Inst, Carry, Zero, uint32_t, __builtin_add_overflow);
        break;
    }
    case 3: {
        OpCheckCarry(Vm, Inst, Carry, Zero, uint64_t, __builtin_add_overflow);
        break;
    }
    }

    SET_FLAGS(Vm, Carry, Zero);
}

void AstroVmSubInst(AstroVm *Vm, AstroVmInst *Inst) {
    bool Carry = false;
    bool Zero = false;
    switch (Inst->Size) {
    case 0: {
        OpCheckCarry(Vm, Inst, Carry, Zero, uint8_t, __builtin_sub_overflow);
        break;
    }
    case 1: {
        OpCheckCarry(Vm, Inst, Carry, Zero, uint16_t, __builtin_sub_overflow);
        break;
    }
    case 2: {
        OpCheckCarry(Vm, Inst, Carry, Zero, uint32_t, __builtin_sub_overflow);
        break;
    }
    case 3: {
        OpCheckCarry(Vm, Inst, Carry, Zero, uint64_t, __builtin_sub_overflow);
        break;
    }
    }

    SET_FLAGS(Vm, Carry, Zero);
}

void AstroVmMulInst(AstroVm *Vm, AstroVmInst *Inst) {
    bool Carry = false;
    bool Zero = false;
    switch (Inst->Size) {
    case 0: {
        OpCheckCarry(Vm, Inst, Carry, Zero, uint8_t, __builtin_mul_overflow);
        break;
    }
    case 1: {
        OpCheckCarry(Vm, Inst, Carry, Zero, uint16_t, __builtin_mul_overflow);
        break;
    }
    case 2: {
        OpCheckCarry(Vm, Inst, Carry, Zero, uint32_t, __builtin_mul_overflow);
        break;
    }
    case 3: {
        OpCheckCarry(Vm, Inst, Carry, Zero, uint64_t, __builtin_mul_overflow);
        break;
    }
    }

    SET_FLAGS(Vm, Carry, Zero);
}

void AstroVmDivInst(AstroVm*, AstroVmInst*) {
    printf("Div Stub!\n");
}

void AstroVmMovInst(AstroVm *Vm, AstroVmInst *Inst) {
    AstroVmSetDst(Vm, Inst, AstroVmGetSrc(Vm, Inst, true), true);
}

void AstroVmJmpInst(AstroVm *Vm, AstroVmInst *Inst) {
    uint8_t CondType = AstroVmRead8(Vm, Vm->Registers[REG_IP]++);
    uint64_t Address = AstroVmGetSrc(Vm, Inst, true);
    // TODO: Handle relative & absolute
    if (CondType & INST_JMP_CARRY) {
        if (Vm->Registers[REG_FLAGS] & REG_FLAGS_CARRY) {
            Vm->Registers[REG_IP] = Address;
            return;
        }
    } else if (CondType & INST_JMP_ZERO) {
        if (Vm->Registers[REG_FLAGS] & REG_FLAGS_ZERO) {
            Vm->Registers[REG_IP] = Address;
            return;
        }
    } else {
        Vm->Registers[REG_IP] = Address;
        return;
    }
}

void AstroVmPushInst(AstroVm *Vm, AstroVmInst *Inst) {
    uint64_t Source = AstroVmGetSrc(Vm, Inst, true);
    AstroVmPush(Vm, Source, Inst->Size);
}

void AstroVmPopInst(AstroVm *Vm, AstroVmInst *Inst) {
    uint64_t Source = AstroVmPop(Vm, Inst->Size);
    AstroVmSetDst(Vm, Inst, Source, true);
}

void AstroVmCallInst(AstroVm *Vm, AstroVmInst *Inst) {
    uint64_t Address = AstroVmGetSrc(Vm, Inst, true);
    // TODO: Handle relative & absolute
    AstroVmPush(Vm, Vm->Registers[REG_IP], 3);
    Vm->Registers[REG_IP] = Address;
}

void AstroVmRetInst(AstroVm *Vm, AstroVmInst *Inst) {
    Vm->Registers[REG_IP] = AstroVmPop(Vm, 3);
}

#define LOGICAL_OP(Vm, Inst, Type, Op)             \
    Type Src = (Type)AstroVmGetSrc(Vm, Inst, true); \
    Type Dst = (Type)AstroVmGetDst(Vm, Inst, false); \
    AstroVmSetDst(Vm, Inst, Dst Op Src, true)       \

void AstroVmAndInst(AstroVm *Vm, AstroVmInst *Inst) {
    switch (Inst->Size) {
    case 0: {
        LOGICAL_OP(Vm, Inst, uint8_t, &);
        break;
    }
    case 1: {
        LOGICAL_OP(Vm, Inst, uint16_t, &);
        break;
    }
    case 2: {
        LOGICAL_OP(Vm, Inst, uint32_t, &);
        break;
    }
    case 3: {
        LOGICAL_OP(Vm, Inst, uint64_t, &);
        break;
    }
    }
}

void AstroVmOrInst(AstroVm *Vm, AstroVmInst *Inst) {
    switch (Inst->Size) {
    case 0: {
        LOGICAL_OP(Vm, Inst, uint8_t, |);
        break;
    }
    case 1: {
        LOGICAL_OP(Vm, Inst, uint16_t, |);
        break;
    }
    case 2: {
        LOGICAL_OP(Vm, Inst, uint32_t, |);
        break;
    }
    case 3: {
        LOGICAL_OP(Vm, Inst, uint64_t, |);
        break;
    }
    }
}

void AstroVmXorInst(AstroVm *Vm, AstroVmInst *Inst) {
    switch (Inst->Size) {
    case 0: {
        LOGICAL_OP(Vm, Inst, uint8_t, ^);
        break;
    }
    case 1: {
        LOGICAL_OP(Vm, Inst, uint16_t, ^);
        break;
    }
    case 2: {
        LOGICAL_OP(Vm, Inst, uint32_t, ^);
        break;
    }
    case 3: {
        LOGICAL_OP(Vm, Inst, uint64_t, ^);
        break;
    }
    }
}

#define LOGICAL_NOT_OP(Vm, Inst, Type)             \
    Type Dst = (Type)AstroVmGetDst(Vm, Inst, false); \
    AstroVmSetDst(Vm, Inst, ~Dst, true)       \

void AstroVmNotInst(AstroVm *Vm, AstroVmInst *Inst) {
    switch (Inst->Size) {
    case 0: {
        LOGICAL_NOT_OP(Vm, Inst, uint8_t);
        break;
    }
    case 1: {
        LOGICAL_NOT_OP(Vm, Inst, uint16_t);
        break;
    }
    case 2: {
        LOGICAL_NOT_OP(Vm, Inst, uint32_t);
        break;
    }
    case 3: {
        LOGICAL_NOT_OP(Vm, Inst, uint64_t);
        break;
    }
    }
}

void AstroVmShlInst(AstroVm *Vm, AstroVmInst *Inst) {
    switch (Inst->Size) {
    case 0: {
        LOGICAL_OP(Vm, Inst, uint8_t, <<);
        break;
    }
    case 1: {
        LOGICAL_OP(Vm, Inst, uint16_t, <<);
        break;
    }
    case 2: {
        LOGICAL_OP(Vm, Inst, uint32_t, <<);
        break;
    }
    case 3: {
        LOGICAL_OP(Vm, Inst, uint64_t, <<);
        break;
    }
    }
    
}

void AstroVmShrInst(AstroVm *Vm, AstroVmInst *Inst) {
    switch (Inst->Size) {
    case 0: {
        LOGICAL_OP(Vm, Inst, uint8_t, >>);
        break;
    }
    case 1: {
        LOGICAL_OP(Vm, Inst, uint16_t, >>);
        break;
    }
    case 2: {
        LOGICAL_OP(Vm, Inst, uint32_t, >>);
        break;
    }
    case 3: {
        LOGICAL_OP(Vm, Inst, uint64_t, >>);
        break;
    }
    }
}

void AstroVmSeiInst(AstroVm *Vm, AstroVmInst *Inst) {
    Vm->Registers[REG_FLAGS] |= REG_FLAGS_INT;
}

void AstroVmSdiInst(AstroVm *Vm, AstroVmInst *Inst) {
    Vm->Registers[REG_FLAGS] &= ~REG_FLAGS_INT;
}

void AstroVmIntInst(AstroVm *Vm, AstroVmInst *Inst) {
    uint8_t Vector = AstroVmGetSrc(Vm, Inst, true);
    AstroVmRaiseInt(Vm, Vector);
}

void AstroVmDumpRegs(AstroVm *Vm) {
    for (int i = 0; i <= 10; i++)
        printf("G%d: 0x%lx\n", i, Vm->Registers[i]);
    printf("SP: 0x%lx\n", Vm->Registers[REG_SP]);
    printf("FP: 0x%lx\n", Vm->Registers[REG_FP]);
    printf("IP: 0x%lx\n", Vm->Registers[REG_IP]);
    printf("Flags: 0x%lx\n", Vm->Registers[REG_FLAGS]);
    printf("PgTbl: 0x%lx\n", Vm->Registers[REG_PGTBL]);
    printf("IVTbl: 0x%lx\n", Vm->Registers[REG_IVTBL]);
}

void AstroVmDestroy(AstroVm *Vm) {
    free(Vm->Ram);
    free(Vm);
}
