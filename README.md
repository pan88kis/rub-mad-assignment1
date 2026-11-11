# rub-mad-assignment1
Flush+Reload side-channel attack against modified GnuPG. Demonstrates cache-timing leakage from shared memory and traces cryptographic activity on Debian 12, x86_64, Intel i7-6700.
# Flush + Reload against GnuPG

## Overview

This project implements a microarchitectural attack demonstration based on Flush+Reload against a modified version of GnuPG. The core idea is to evict a cache line using `clflush` and then measure the access time to a shared memory address. If the victim accesses that address, the cache line is reloaded and the read latency becomes noticeably smaller, allowing the attacker to infer that a sensitive function was executed.

The repository contains:

- `monitor.c`: monitors a target address to detect recent access.
- `fr_trace.c`: measures access latency on two target addresses to build a behavioral trace.
- `gpg_attack.c`: orchestrates key generation and the attack flow.
- `flush_reload.c` and `flush_reload.h`: implement the Flush+Reload logic and utility functions.
- `victimshim.c`: preload library used to enforce the victim behavior.
- `victim_vm/`: target environment containing a modified GnuPG version and testing tools.

## Project objective

The goal was to demonstrate that a local attacker, by sharing memory pages with a sensitive application, can observe accesses to critical functions inside the cryptographic code. In this case, the target is the key computation / modular exponentiation routines in GnuPG, where access to specific memory addresses reveals the execution of cryptographic operations.

The main attack flow is as follows:

1. Generate a GnuPG key in an isolated environment.
2. Launch the victim process with `LD_PRELOAD`.
3. Flush the targeted cache lines.
4. Measure access timing using `rdtsc` and `clflush`.
5. Detect access to the monitored function.
6. Store the trace in `gnupg.txt`.

## Build

```bash
make
```

This produces, among others:

- `monitor`
- `fr_trace`
- `gpg_attack`
- `libvictimshim.so`

## Test machine

The tests were performed on a local Debian 12 Linux machine with an Intel x86_64 architecture compatible with Flush+Reload attacks.

### Machine characteristics

```text
OS: Debian GNU/Linux 12 (bookworm)
Architecture: x86_64
CPU: Intel(R) Core(TM) i7-6700 CPU @ 3.40GHz
CPU(s): 8
Thread(s) per core: 2
Core(s) per socket: 4
Socket(s): 1
RAM: 31 GiB
Virtualization: VT-x enabled
```

### Verification commands

```bash
cat /etc/os-release
lscpu
free -h
nproc
grep -m 1 'model name' /proc/cpuinfo
grep MemTotal /proc/meminfo
```

### Observed output

```text
PRETTY_NAME="Debian GNU/Linux 12 (bookworm)"
NAME="Debian GNU/Linux"
VERSION_ID="12"
VERSION="12 (bookworm)"
ID=debian

Architecture: x86_64
CPU(s): 8
Model name: Intel(R) Core(TM) i7-6700 CPU @ 3.40GHz
MemTotal: 32743380 kB
```

## Technical implementation

The key logic relies on:

- `clflush` to evict the targeted cache line,
- `rdtsc` to measure access latency,
- `lfence` / `mfence` to order instructions and avoid reordering,
- `mmap` on the victim binary to obtain pointers to the targeted functions.

The core of the attack is the `flush_reload()` function in `flush_reload.c`, which measures the difference in timing between:

- an access to a flushed cache line,
- an access to a cache line still resident in cache.

## Expected result

The output stored in `gnupg.txt` contains sequences of latency values. Recent accesses to sensitive functions appear as lower access times, allowing the attacker to identify periods when the victim executed cryptographic computation.

## Note

This project is designed as an academic demonstration of cache-based microarchitectural vulnerabilities and side-channel attacks, in the context of a lab environment and with the standard precautions for test systems.
