#include "dev.h"
#include <raylib.h>
#include <stdio.h>
#include "ivt.h"
#include "mem.h"

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

void vm_init_devices(vm_t *vm) {
}

void vm_update_devices(vm_t *vm) {
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