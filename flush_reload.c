#define _GNU_SOURCE

#include "flush_reload.h"

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

static inline uint64_t read_tsc(void) {
    unsigned int low;
    unsigned int high;
    __asm__ __volatile__("rdtsc" : "=a"(low), "=d"(high));
    return ((uint64_t)high << 32) | low;
}

void *map(char *file_name, uint64_t offset) {
    int fd = open(file_name, O_RDONLY);
    if (fd < 0) {
        return NULL;
    }

    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size <= 0) {
        close(fd);
        return NULL;
    }

    uint64_t page_size_u = (uint64_t)page_size;
    uint64_t page_offset = offset & ~(page_size_u - 1ULL);
    uint64_t in_page_offset = offset - page_offset;

    void *mapped = mmap(NULL, (size_t)page_size, PROT_READ, MAP_PRIVATE, fd, (off_t)page_offset);
    close(fd);
    if (mapped == MAP_FAILED) {
        return NULL;
    }

    return (void *)((char *)mapped + in_page_offset);
}

uint32_t flush_reload(void *p) {
    uint64_t start, end;

    asm volatile("mfence\n\t"
                 "lfence\n\t"
                 :
                 :
                 : "memory");

    asm volatile("clflush (%0)\n\t" : : "r"(p) : "memory");
    asm volatile("lfence\n\t" : : : "memory");

    start = read_tsc();
    volatile uint8_t dummy = *(volatile uint8_t *)p;
    (void)dummy;

    asm volatile("lfence\n\t" : : : "memory");
    end = read_tsc();

    return (uint32_t)(end - start);
}

int monitor(void *p, uint32_t period, uint64_t maxwait) {
    uint64_t start = read_tsc();
    uint64_t deadline = start + maxwait;

    while (read_tsc() < deadline) {
        uint32_t latency = flush_reload(p);
        if (latency < 500U) {
            return 1;
        }

        uint64_t now = read_tsc();
        uint64_t target = now + (uint64_t)period;
        while (read_tsc() < target) {
            asm volatile("pause\n\t" : : : "memory");
        }
    }

    return 0;
}

void fr_trace(void *p1, void *p2, uint32_t period, uint32_t count, uint32_t *trace) {
    for (uint32_t i = 0; i < count; ++i) {
        trace[2 * i] = flush_reload(p1);
        trace[2 * i + 1] = flush_reload(p2);

        if (i + 1 < count) {
            uint64_t target = read_tsc() + (uint64_t)period;
            while (read_tsc() < target) {
                asm volatile("pause\n\t" : : : "memory");
            }
        }
    }
}
