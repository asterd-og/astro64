#include "dev.h"
#include <raylib.h>
#include <stdio.h>
#include <pthread.h>
#include "ivt.h"
#include "mem.h"
#include "main.h"
#include "devices/disk.h"

static const uint8_t kb_map_keys[] = {
    0,
    [39]='"',
    [44]='<',
    '_', '>', '?', ')', '!', '@', '#', '$', '%', '"', '&', '*', '(', 0,
    ':', 0, '+', 0, 0, 0, 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
    'K',
    'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', '{', '|', '}', '`', ' '
};

pthread_t kb_dev_thread;

void *vm_kb_dev_loop(void *arg) {
    vm_t *vm = (vm_t*)arg;
    while (vm_running) {
        int key = GetKeyPressed();
        if (key) {
#ifdef DEVICE_DEBUG
           printf("Pressed %d %c\n", key, kb_map_keys[key]);
#endif
            vm_write32(vm, 0xA000, key);
            vm_write8(vm, 0xA004, 1);
            vm_raise(vm, 0);
        } else {
            for (int i = 0; i < 512; i++) {
                if (IsKeyReleased(i)) {
#ifdef DEVICE_DEBUG
                    printf("Released %d %c\n", i, kb_map_keys[i]);
#endif
                    vm_write32(vm, 0xA000, i);
                    vm_write8(vm, 0xA004, 0);
                    vm_raise(vm, 0);
                    break;
                }
            }
        }
    }
    return NULL;
}

void vm_init_devices(vm_t *vm) {
    vm_disk_init(vm);
    pthread_create(&kb_dev_thread, NULL, vm_kb_dev_loop, vm);
}

void vm_unload_devices() {
    vm_disk_unload();
    pthread_cancel(kb_dev_thread);
}
