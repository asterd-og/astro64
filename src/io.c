#include "io.h"
#include "mem.h"

void AstroVmRegisterDevice(AstroVm *Vm, uint64_t Id, uint64_t Address) {
    uint64_t Offset = 0x1000 + (16 * (Id - 1));
    if (Id > 2)
        Offset -= 16;
    AstroVmWrite64(Vm, Offset, Id);
    AstroVmWrite64(Vm, Offset + 8, Address);
}

