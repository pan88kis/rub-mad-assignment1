CC = gcc
CFLAGS = -O2 -Wall -Wextra -std=gnu11
LDFLAGS =

all: monitor fr_trace gpg_attack libvictimshim.so

monitor: monitor.o flush_reload.o
	$(CC) $(CFLAGS) -o $@ monitor.o flush_reload.o

fr_trace: fr_trace.o flush_reload.o
	$(CC) $(CFLAGS) -o $@ fr_trace.o flush_reload.o

gpg_attack: gpg_attack.o flush_reload.o
	$(CC) $(CFLAGS) -o $@ gpg_attack.o flush_reload.o

libvictimshim.so: victimshim.c
	$(CC) $(CFLAGS) -shared -fPIC -ldl -o $@ victimshim.c

monitor.o: monitor.c flush_reload.h
	$(CC) $(CFLAGS) -c monitor.c

fr_trace.o: fr_trace.c flush_reload.h
	$(CC) $(CFLAGS) -c fr_trace.c

gpg_attack.o: gpg_attack.c flush_reload.h
	$(CC) $(CFLAGS) -c gpg_attack.c

clean:
	rm -f *.o monitor fr_trace gpg_attack libvictimshim.so gnupg.txt trace.out

.PHONY: all clean
