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
#ifndef ZPU_MEM_H
#define ZPU_MEM_H

#include <stdint.h>

#define ZPU_MEM_BAD     0xFEFEFEFE

typedef struct _zpu_mem_
{
    const char*         name;
    struct _zpu_mem_*   next;
    void*               physical_base;
    uint32_t            virtual_base;
    uint32_t            size;
} zpu_mem_t;

#define zpu_mem_set_physical_base(zpu_mem,b)    ((zpu_mem)->physical_base = (b)) 
#define zpu_mem_set_virtual_base(zpu_mem,b)     ((zpu_mem)->virtual_base = (b)) 
#define zpu_mem_set_size(zpu_mem,s)             ((zpu_mem)->size = (s)) 
#define zpu_mem_get_size(zpu_mem)               ((zpu_mem)->size) 
#define zpu_mem_set_next(zpu_mem,n)             ((zpu_mem)->next = (n)) 
#define zpu_mem_set_name(zpu_mem,n)             ((zpu_mem)->name = (n))

extern void         zpu_mem_init( zpu_mem_t* zpu_mem_root, 
                                  zpu_mem_t* zpu_mem_seg, 
                                  const char* name, 
                                  void* physical_base, 
                                  uint32_t virtual_base, 
                                  uint32_t size );

extern uint32_t     zpu_mem_get_uint32( zpu_mem_t* zpu_mem, uint32_t va );
extern uint16_t     zpu_mem_get_uint16( zpu_mem_t* zpu_mem, uint32_t va );
extern uint8_t      zpu_mem_get_uint8( zpu_mem_t* zpu_mem, uint32_t va );

extern void         zpu_mem_set_uint32( zpu_mem_t* zpu_mem, uint32_t va, uint32_t w );
extern void         zpu_mem_set_uint16( zpu_mem_t* zpu_mem, uint32_t va, uint16_t w );
extern void         zpu_mem_set_uint8( zpu_mem_t* zpu_mem, uint32_t va, uint8_t w );

#endif

