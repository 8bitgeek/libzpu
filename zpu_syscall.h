#ifndef ZPU_SYSCALL_H
#define ZPU_SYSCALL_H

#include <zpu.h>

// syscall ID numbers
#define SYS_READ  4
#define SYS_WRITE 5

void zpu_syscall(zpu_t* zpu);

#endif