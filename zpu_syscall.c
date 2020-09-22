#include <stdio.h>
#include <stdlib.h>
#include <termio.h>

#include <zpu_syscall.h>
#include <zpu_mem.h>

static int tty_getchar();

void syscall(zpu_t* zpu, uint32_t sp)
{
    // int returnAdd = zpu_mem_get_uint32( zpu_get_mem(zpu), sp + 0);
    // int errNoAdd = zpu_mem_get_uint32( zpu_get_mem(zpu), sp + 4);
    int sysCallId = zpu_mem_get_uint32( zpu_get_mem(zpu), sp + 8);
    // int fileNo = zpu_mem_get_uint32( zpu_get_mem(zpu), sp + 12);
    int charIndex = zpu_mem_get_uint32( zpu_get_mem(zpu), sp + 16);
    int stringLength = zpu_mem_get_uint32( zpu_get_mem(zpu), sp + 20);
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
                zpu_mem_set_uint8( zpu_get_mem(zpu), charIndex++, tty_getchar());
            }
            // Return value via R0 (AKA memory address 0)
            zpu_mem_set_uint32( zpu_get_mem(zpu), 0, stringLength);
            break;
    }
}

int tty_fix()
{
    return 0;
}

static int tty_getchar()
{
    return getchar();
}
