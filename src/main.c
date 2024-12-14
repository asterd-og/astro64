#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "vm.h"
#include "mem.h"

int main(int argc, char **argv) {
    if (argv < 2) {
        printf("Expected Program Binary.\n");
        exit(1);
    }

    AstroVm *Vm = AstroVmInitialise(2097152);

    FILE *ProgramFile = fopen(argv[1], "rb");
    fseek(ProgramFile, 0, SEEK_END);
    size_t Size = ftell(ProgramFile);
    rewind(ProgramFile);
    uint8_t *Buffer = (uint8_t*)malloc(Size);
    fread(Buffer, 1, Size, ProgramFile);

    AstroVmLoadProgram(Vm, Buffer, Size);

    while (Vm->Registers[REG_IP] < Size) {
        AstroVmStep(Vm);
    }

    AstroVmDumpRegs(Vm);

    AstroVmDestroy(Vm);
    return 0;
}

