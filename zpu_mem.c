/****************************************************************************
 * Copyright (c) 2020 Mike Sharkey <mike@pikeaero.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <zpu_mem.h>

static void         zpu_mem_append( zpu_mem_t* zpu_mem_root, zpu_mem_t* zpu_mem_seg );
static zpu_mem_t*   zpu_mem_seg_v( zpu_mem_t* zpu_mem_root, uint32_t va );
static void*        zpu_va_to_pa( zpu_mem_t* zpu_mem, uint32_t va );


extern void zpu_mem_init( zpu_mem_t* zpu_mem_root, 
                          zpu_mem_t* zpu_mem_seg, 
                          const char* name, 
                          void* physical_base, 
                          uint32_t virtual_base, 
                          uint32_t size )
{
    if ( zpu_mem_root )
    {
        zpu_mem_seg->next=NULL;
        zpu_mem_append( zpu_mem_root, zpu_mem_seg );
    }
    if ( zpu_mem_seg )
    {
        zpu_mem_seg->name = name;
        zpu_mem_seg->physical_base = physical_base;
        zpu_mem_seg->virtual_base = virtual_base;
        zpu_mem_seg->size = size;
    }
}


extern uint32_t zpu_mem_get_uint32( zpu_mem_t* zpu_mem, uint32_t va )
{
    // FIXME --- this puke ....
    // The PHI platform UART status port
    if (va == 0x80000024)
    {
	    return (0x100);
    }
    // The ??? platform UART status port
    if (va == 0x080A000C)
    {
	    return (0x100);
    }


    zpu_mem_t* zpu_seg = zpu_mem_seg_v( zpu_mem, va );
    if ( zpu_seg )
    {
        void* pa = zpu_va_to_pa( zpu_seg, va );
        uint32_t* p = (uint32_t*)pa;
        return *p;
    }
    printf( "segv %04X\n", va);
    return ZPU_MEM_BAD;
}

extern uint16_t zpu_mem_get_uint16( zpu_mem_t* zpu_mem, uint32_t va )
{
    zpu_mem_t* zpu_seg = zpu_mem_seg_v( zpu_mem, va );
    if ( zpu_seg )
    {
        void* pa = zpu_va_to_pa( zpu_seg, va ^ 0x02 );
        uint16_t* p = (uint16_t*)pa;
        return *p;
    }
    printf( "segv %04X\n", va);
    return ZPU_MEM_BAD&0xFFFF;
}

extern uint8_t  zpu_mem_get_uint8( zpu_mem_t* zpu_mem, uint32_t va )
{
    zpu_mem_t* zpu_seg = zpu_mem_seg_v( zpu_mem, va );
    if ( zpu_seg )
    {
        void* pa = zpu_va_to_pa( zpu_seg, va ^ 0x03 );
        uint8_t* p = (uint8_t*)pa;
        return *p;
    }
    printf( "segv %04X\n", va);
    return ZPU_MEM_BAD&0xFF;
}


extern void zpu_mem_set_uint32( zpu_mem_t* zpu_mem, uint32_t va, uint32_t w )
{
    // FIXME - THis puke...
    // The PHI platform UART port
    if (va == 0x80000024)
    {
    	printf("%c", (char)w);
    	return;
    }
    // The ??? platform UART port
    if (va == 0x080A000C)
    {
    	printf("%c", (char)w);
    	return;
    }


    zpu_mem_t* zpu_seg = zpu_mem_seg_v( zpu_mem, va );
    if ( zpu_seg )
    {
        void* pa = zpu_va_to_pa( zpu_seg, va );
        uint32_t* p = (uint32_t*)pa;
        *p = w;
        return;
    }
    printf( "segv %04X\n", va);
}

extern void zpu_mem_set_uint16( zpu_mem_t* zpu_mem, uint32_t va, uint16_t w )
{
    zpu_mem_t* zpu_seg = zpu_mem_seg_v( zpu_mem, va );
    if ( zpu_seg )
    {
        void* pa = zpu_va_to_pa( zpu_seg, va ^ 0x02 );
        uint16_t* p = (uint16_t*)pa;
        *p = w;
        return;
    }
    printf( "segv %04X\n", va);
}

extern void zpu_mem_set_uint8( zpu_mem_t* zpu_mem, uint32_t va, uint8_t w )
{
    zpu_mem_t* zpu_seg = zpu_mem_seg_v( zpu_mem, va );
    if ( zpu_seg )
    {
        void* pa = zpu_va_to_pa( zpu_seg, va ^ 0x03 );
        uint8_t* p = (uint8_t*)pa;
        *p = w;
        return;
    }
    printf( "segv %04X\n", va);
}


static void zpu_mem_append( zpu_mem_t* zpu_mem_root, zpu_mem_t* zpu_mem_seg )
{
    for(zpu_mem_t* next=zpu_mem_root; next; next=next->next)
    {
        if ( next->next == NULL )
        {
            next->next = zpu_mem_seg;
            return;
        }
    }
}

static zpu_mem_t* zpu_mem_seg_v( zpu_mem_t* zpu_mem_root, uint32_t va )
{
    for(zpu_mem_t* mem_seg=zpu_mem_root; mem_seg; mem_seg=mem_seg->next)
    {
        if ( mem_seg->virtual_base <= va && va < mem_seg->virtual_base + mem_seg->size )
        {
            return mem_seg;
        }
    }
    return NULL;
}

static void* zpu_va_to_pa( zpu_mem_t* zpu_mem, uint32_t va )
{
    if ( zpu_mem )
    {
        uint32_t delta = va - zpu_mem->virtual_base;
        return ((uint8_t*)zpu_mem->physical_base) + delta;
    }
    return (uint32_t*)ZPU_MEM_BAD;
}

