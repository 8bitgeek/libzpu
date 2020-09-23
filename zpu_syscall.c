#include <stdio.h>
#include <stdlib.h>
#include <termio.h>

#include <zpu_syscall.h>
#include <zpu_mem.h>

void zpu_syscall(zpu_t* zpu)
{
    // int returnAdd = zpu_mem_get_uint32( zpu_get_mem(zpu), zpu_get_sp(zpu) + 0);
    // int errNoAdd = zpu_mem_get_uint32( zpu_get_mem(zpu), zpu_get_sp(zpu) + 4);
    int sysCallId = zpu_mem_get_uint32( zpu_get_mem(zpu), zpu_get_sp(zpu) + 8);
    // int fileNo = zpu_mem_get_uint32( zpu_get_mem(zpu), zpu_get_sp(zpu) + 12);
    int charIndex = zpu_mem_get_uint32( zpu_get_mem(zpu), zpu_get_sp(zpu) + 16);
    int stringLength = zpu_mem_get_uint32( zpu_get_mem(zpu), zpu_get_sp(zpu) + 20);
    switch (sysCallId)
    {
        case SYS_WRITE:
            for (int i = 0; i < stringLength; i++)
            {
                putchar(zpu_mem_get_uint8( zpu_get_mem(zpu), charIndex++));
            }
            // Return value via R0 (AKA memory address 0)
            zpu_mem_set_uint32( zpu_get_mem(zpu), 0, stringLength);
            break;
        case SYS_READ:
            for (int i = 0; i < stringLength; i++)
            {
                zpu_mem_set_uint8( zpu_get_mem(zpu), charIndex++, getchar());
            }
            // Return value via R0 (AKA memory address 0)
            zpu_mem_set_uint32( zpu_get_mem(zpu), 0, stringLength);
            break;
    }
}
