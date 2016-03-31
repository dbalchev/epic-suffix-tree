#ifndef OPTIMIZE_H
#define OPTIMIZE_H

#include <sys/mman.h>
#include <unistd.h>
#define MY_ALLOC(SZ)      ::malloc(SZ)//mmap(0, SZ, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#define MY_FREE(ADDR, SZ) ::free(ADDR)  //munmap(ADDR, SZ)

#define THROW(X) {std::cerr << X << std::endl; _exit(1);}
#ifdef __GNUC__
#define likely(x)       __builtin_expect((long int)(x),1)
#define unlikely(x)     __builtin_expect((long int)(x),0)
#else 
#define likely(x)       (x)
#define unlikely(x)     (x)
#endif

#endif
