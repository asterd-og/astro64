#include "disk.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "../mem.h"
#include "../main.h"

disk_t *disks[32];
int disk_count = 0;

pthread_t disk_dev_thread;

int vm_disk_load(char *file_name, char *disk_name) {
    FILE *fp = fopen(file_name, "w+");
    if (!fp)
        return 1;
    disk_t *disk = (disk_t*)malloc(sizeof(disk_t));
    memcpy(disk->name, disk_name, 32);
    disk->fp = fp;
    disk->write = false;
    fseek(fp, 0, SEEK_END);
    disk->sector_count = ftell(fp) / DISK_SECTOR_SIZE;
    rewind(fp);
    disk->flags = DISK_RDY;
    disks[disk_count++] = disk;
    return 0;
}

int vm_disk_read(vm_t *vm, disk_t *disk, uint64_t sector, uint64_t sector_off, uint64_t buffer_address) {
    uint8_t *ptr = vm_ptr(vm, buffer_address + (sector_off * DISK_SECTOR_SIZE));
    fseek(disk->fp, sector * DISK_SECTOR_SIZE, SEEK_SET);
    if (fread(ptr, DISK_SECTOR_SIZE, 1, disk->fp) < DISK_SECTOR_SIZE && ferror(disk->fp)) {
        printf("Disk Error: Disk reading returned %d.\n", ferror(disk->fp));
        return 1;
    }
    return 0;
}

int vm_disk_write(vm_t *vm, disk_t *disk, uint64_t sector, uint64_t sector_off, uint64_t buffer_address) {
    uint8_t *ptr = vm_ptr(vm, buffer_address + (sector_off * DISK_SECTOR_SIZE));
    fseek(disk->fp, sector * DISK_SECTOR_SIZE, SEEK_SET);
    if (fwrite(ptr, DISK_SECTOR_SIZE, 1, disk->fp) < DISK_SECTOR_SIZE && ferror(disk->fp)) {
        printf("Disk Error: Disk writing returned %d.\n", ferror(disk->fp));
        return 1;
    }
    return 0;
}

int vm_disk_operation(vm_t *vm, disk_t *disk, uint64_t sector, uint64_t sector_off, uint64_t buffer_address) {
    if (disk->write) return vm_disk_write(vm, disk, sector, sector_off, buffer_address);
    return vm_disk_read(vm, disk, sector, sector_off, buffer_address);
}

void *vm_disk_update(void *arg) {
    vm_t *vm = (vm_t*)arg;
    bool operating = false;
    int disk_operating = 0;
    uint64_t buffer = 0;
    uint64_t sector = 0;
    uint64_t end_sector = 0;
    vm_write8(vm, DISK_STATUS, 0);
    while (vm_running) {
        if (operating) {
            disk_t *disk = disks[disk_operating];
            if (sector < end_sector) {
                if (vm_disk_operation(vm, disk, sector, (end_sector - 1) - sector, buffer)) {
                    vm_write8(vm, DISK_STATUS, DISK_ERR);
                    operating = false;
                    continue;
                }
                sector++;
            } else {
                if (disk->write)
                    fflush(disk->fp);
                vm_write8(vm, DISK_STATUS, DISK_DONE);
                operating = false;
            }
            continue;
        }
        uint8_t command = vm_read8(vm, DISK_COMMAND);
        if (command) {
            vm_write8(vm, DISK_COMMAND, 0);
            uint32_t select = vm_read32(vm, DISK_SELECT);
            disk_t *disk = disks[select];
            if (command == DISK_READ || command == DISK_WRITE) {
                operating = true;
                disk->write = command == DISK_WRITE;
                sector = vm_read64(vm, DISK_SECTOR);
                end_sector = sector + vm_read64(vm, DISK_COUNT);
                buffer = vm_read64(vm, DISK_BUFFER);
                vm_write8(vm, DISK_STATUS, DISK_BSY);
                continue;
            }
            printf("Warning: Unhandled disk command %d.\n", command);
        }
    }
}

void vm_disk_init(vm_t *vm) {
    pthread_create(&disk_dev_thread, NULL, vm_disk_update, vm);
}

void vm_disk_unload() {
    pthread_cancel(disk_dev_thread);
}