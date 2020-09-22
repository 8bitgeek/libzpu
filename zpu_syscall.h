#ifndef ZPU_SYSCALL_H
#define ZPU_SYSCALL_H

#include <stdint.h>
#include <zpu.h>

// syscall ID numbers
#define SYS_READ  4
#define SYS_WRITE 5

void syscall(zpu_t* zpu, uint32_t sp);

#endif