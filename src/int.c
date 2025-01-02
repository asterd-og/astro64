#include "int.h"
#include "mem.h"
#include <stdio.h>

void AstroVmRaiseInt(AstroVm *Vm, uint8_t Vector) {
    if (!(Vm->Registers[REG_FLAGS] & REG_FLAGS_INT))
        return;
    AstroVmIVTable *Tbl = (AstroVmIVTable*)AstroVmGetPtr(Vm, Vm->Registers[REG_IVTBL]);
    if (Vector > Tbl->Count - 1)
        return;
    AstroVmInt *Int = (AstroVmInt*)AstroVmGetPtr(Vm, (uint64_t)(Tbl->Table + Vector));
    // TODO: Handle different access privileges
    if (!(Int->Flags & 1))
        return; // Interrupt not present
    Vm->QueuedVector = Int;
    return;
}

void AstroVmRunQueuedInt(AstroVm *Vm) {
    if (!Vm->QueuedVector)
        return;
    AstroVmPush(Vm, Vm->Registers[REG_IP], 3);
    Vm->Registers[REG_IP] = Vm->QueuedVector->Handler;
    Vm->QueuedVector = NULL;
}

