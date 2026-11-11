#define _GNU_SOURCE

#include <dlfcn.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int (*real_open)(const char *pathname, int flags, ...) = NULL;

int open(const char *pathname, int flags, ...) {
    mode_t mode = 0;
    if (real_open == NULL) {
        real_open = (int (*)(const char *, int, ...))dlsym(RTLD_NEXT, "open");
    }

    if (pathname == NULL) {
        if (flags & O_CREAT) {
            va_list ap;
            va_start(ap, flags);
            mode = va_arg(ap, mode_t);
            va_end(ap);
            return real_open(pathname, flags, mode);
        }
        return real_open(pathname, flags);
    }

    if (strcmp(pathname, "/home/victim/trigger") == 0) {
        pathname = "/home/victim/trigger";
    }

    if (flags & O_CREAT) {
        va_list ap;
        va_start(ap, flags);
        mode = va_arg(ap, mode_t);
        va_end(ap);
        return real_open(pathname, flags, mode);
    }

    return real_open(pathname, flags);
}

int open64(const char *pathname, int flags, ...) {
    mode_t mode = 0;
    if (real_open == NULL) {
        real_open = (int (*)(const char *, int, ...))dlsym(RTLD_NEXT, "open");
    }

    if (pathname == NULL) {
        if (flags & O_CREAT) {
            va_list ap;
            va_start(ap, flags);
            mode = va_arg(ap, mode_t);
            va_end(ap);
            return real_open(pathname, flags, mode);
        }
        return real_open(pathname, flags);
    }

    if (strcmp(pathname, "/home/victim/trigger") == 0) {
        pathname = "/home/eldor/Schreibtisch/Master/1. Fachsemester/Microarchitectural Attacks and Defenses/Assignment/victim_vm/trigger";
    }

    if (flags & O_CREAT) {
        va_list ap;
        va_start(ap, flags);
        mode = va_arg(ap, mode_t);
        va_end(ap);
        return real_open(pathname, flags, mode);
    }

    return real_open(pathname, flags);
}
