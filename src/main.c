#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "vm.h"
#include "mem.h"
#include "io.h"
#include "fb.h"
#include <pthread.h>

AstroVm *Vm;
size_t ProgramSize;

void *CpuThread(void*) {
    while (Vm->Registers[REG_IP] < ProgramSize) {
        AstroVmStep(Vm, false);
    }
    return NULL;
}

int main(int argc, char **argv) {
    if (argv < 2) {
        printf("Expected Program Binary.\n");
        exit(1);
    }

    size_t RamSize = 10 * 1024 * 1024;
    Vm = AstroVmInitialise(RamSize);

    // First 4 kb is program data

    // Serial
    AstroVmRegisterDevice(Vm, 1, 0xA000);
    DeviceFbRegister(Vm, 800, 600);

    FILE *ProgramFile = fopen(argv[1], "rb");
    fseek(ProgramFile, 0, SEEK_END);
    size_t Size = ftell(ProgramFile);
    rewind(ProgramFile);
    uint8_t *Buffer = (uint8_t*)malloc(Size);
    fread(Buffer, 1, Size, ProgramFile);
    ProgramSize = Size;

    AstroVmLoadProgram(Vm, Buffer, Size);

    pthread_t Thread;
    pthread_create(&Thread, NULL, CpuThread, NULL);
    while (!DeviceFbFinished) {
        DeviceFbUpdate(Vm);
        if (Vm->Ram[0xA000] != 0) {
            fprintf(stderr, "%c", Vm->Ram[0xA000]);
            Vm->Ram[0xA001] |= 1;
            Vm->Ram[0xA000] = 0;
        }
    }

    printf("\n-----DUMP-----\n");
    AstroVmDumpRegs(Vm);

    AstroVmDestroy(Vm);
    return 0;
}

