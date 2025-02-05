#include "dev.h"
#include <raylib.h>
#include <stdio.h>
#include <pthread.h>
#include "ivt.h"
#include "mem.h"
#include "main.h"
#include "devices/disk.h"
#include "devices/kb.h"

void vm_init_devices(vm_t *vm) {
    vm_disk_init(vm);
    vm_kb_init(vm);
}

void vm_unload_devices() {
    vm_disk_unload();
    vm_kb_unload();
}
