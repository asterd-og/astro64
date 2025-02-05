#include "kb.h"
#include "../mem.h"
#include "../ivt.h"
#include "../main.h"
#include <string.h>
#include <raylib.h>
#include <pthread.h>

pthread_t kb_dev_thread;

typedef struct {
    uint32_t key;
    uint8_t status;
} vm_kb_event_t;

vm_kb_event_t vm_kb_fifo[64];
int vm_kb_fifo_cnt = 0;

void vm_kb_send_data(vm_t *vm, uint32_t key, uint8_t status) {
    if (vm->ram[I_EOI] == 0) {
        vm_write32(vm, KB_KEY, key);
        vm_write8(vm, KB_STATUS, status);
        vm_raise(vm, KB_INT);
        return;
    }
    // Add to FIFO
    if (vm_kb_fifo_cnt == 64) {
        return;
    }
    vm_kb_fifo[vm_kb_fifo_cnt].key = key;
    vm_kb_fifo[vm_kb_fifo_cnt].status = status;
    vm_kb_fifo_cnt++;
}

void vm_kb_run_fifo(vm_t *vm) {
    if (vm_kb_fifo_cnt == 0) return;
    if (vm->ram[I_EOI] == 1) return;
    uint32_t key = vm_kb_fifo[0].key;
    uint8_t status = vm_kb_fifo[0].status;
    memmove(vm_kb_fifo, vm_kb_fifo + 1, (vm_kb_fifo_cnt - 1) * sizeof(vm_kb_event_t));
    vm_kb_fifo_cnt--;
    vm_write32(vm, KB_KEY, key);
    vm_write8(vm, KB_STATUS, status);
    vm_raise(vm, KB_INT);
}

void *vm_kb_update(void *arg) {
    vm_t *vm = (vm_t*)arg;
    while (vm_running) {
        vm_kb_run_fifo(vm);
        int key = GetKeyPressed();
        if (key) {
            vm_kb_send_data(vm, key, 1);
        } else {
            for (int i = 0; i < 512; i++) {
                if (IsKeyReleased(i)) {
                    vm_kb_send_data(vm, i, 0);
                    break;
                }
            }
        }
    }
    return NULL;
}

void vm_kb_init(vm_t *vm) {
    pthread_create(&kb_dev_thread, NULL, vm_kb_update, vm);
}

void vm_kb_unload() {
    pthread_cancel(kb_dev_thread);
}