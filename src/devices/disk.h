#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include "../cpu.h"

// Each sector is 1024 bytes in size

#define DISK_RDY 1
#define DISK_BSY 2
#define DISK_ERR 4
#define DISK_DONE 8

#define DISK_READ 1
#define DISK_WRITE 2

#define DISK_SECTOR_SIZE 1024

enum {
    DISK_SELECT = 0x20000, // 4 bytes
    DISK_STATUS = 0x20004, // 2 bytes
    DISK_COMMAND = 0x20006, // 1 byte
    DISK_SECTOR = 0x20008, // 8 bytes
    DISK_COUNT = 0x20010, // 8 bytes
    DISK_BUFFER = 0x20018, // 8 bytes
};

typedef struct {
    char name[32];
    FILE *fp;
    bool write;
    uint64_t sector_count;
    uint16_t flags; // ready, busy, fail
} disk_t;

void vm_disk_init(vm_t *vm);
int vm_disk_load(char *file_name, char *disk_name);
void vm_disk_unload();