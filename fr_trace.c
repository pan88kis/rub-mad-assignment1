#include "flush_reload.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void usage(const char *prog) {
    fprintf(stderr, "Usage: %s [-p period] [-c count] <file> <offset1> <offset2>\n", prog);
}

int main(int argc, char **argv) {
    uint32_t period = 10000U;
    uint32_t count = 1000U;
    int i = 1;

    while (i < argc && argv[i][0] == '-') {
        if (strcmp(argv[i], "-p") == 0) {
            if (i + 1 >= argc) {
                usage(argv[0]);
                return 1;
            }
            period = (uint32_t)strtoul(argv[++i], NULL, 0);
        } else if (strcmp(argv[i], "-c") == 0) {
            if (i + 1 >= argc) {
                usage(argv[0]);
                return 1;
            }
            count = (uint32_t)strtoul(argv[++i], NULL, 0);
        } else {
            usage(argv[0]);
            return 1;
        }
        ++i;
    }

    if (argc - i != 3) {
        usage(argv[0]);
        return 1;
    }

    char *file_name = argv[i];
    uint64_t offset1 = strtoull(argv[i + 1], NULL, 0);
    uint64_t offset2 = strtoull(argv[i + 2], NULL, 0);

    void *p1 = map(file_name, offset1);
    void *p2 = map(file_name, offset2);
    if (!p1 || !p2) {
        perror("map");
        return 1;
    }

    uint32_t *trace = calloc((size_t)2U * (size_t)count, sizeof(uint32_t));
    if (!trace) {
        perror("calloc");
        return 1;
    }

    fr_trace(p1, p2, period, count, trace);

    for (uint32_t j = 0; j < count; ++j) {
        printf("%" PRIu32 " %" PRIu32 "\n", trace[2U * j], trace[2U * j + 1U]);
    }

    free(trace);
    return 0;
}
