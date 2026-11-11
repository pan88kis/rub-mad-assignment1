#include "flush_reload.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static inline uint64_t read_tsc(void) {
    unsigned int low;
    unsigned int high;
    __asm__ __volatile__("rdtsc" : "=a"(low), "=d"(high));
    return ((uint64_t)high << 32) | low;
}

static void usage(const char *prog) {
    fprintf(stderr, "Usage: %s <filename> <offset> <period>\n", prog);
}

int main(int argc, char **argv) {
    if (argc != 4) {
        usage(argv[0]);
        return 1;
    }

    char *file_name = argv[1];
    uint64_t offset = strtoull(argv[2], NULL, 0);
    uint32_t period = (uint32_t)strtoul(argv[3], NULL, 0);

    void *p = map(file_name, offset);
    if (p == NULL) {
        perror("map");
        return 1;
    }

    const uint64_t max_total_cycles = 10ULL * 1000ULL * 1000ULL * 1000ULL;
    uint64_t hits[2000];
    size_t hit_count = 0;
    uint64_t start = read_tsc();

    while (hit_count < 2000 && (read_tsc() - start) < max_total_cycles) {
        if (monitor(p, period, 5000000ULL) == 1) {
            hits[hit_count++] = read_tsc();
        }
    }

    for (size_t i = 0; i < hit_count; ++i) {
        printf("%" PRIu64 "\n", hits[i]);
    }

    return 0;
}
