
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
#ifndef ZPU_H
#define ZPU_H


#if defined(_CARIBOU_RTOS_)
    #include <caribou/kernel/types.h> 
#else
    #include <stdint.h>
    #include <stdbool.h>
    #include <stdlib.h>
#endif

#include <zpu_mem.h>

typedef struct _zpu_
{
    zpu_mem_t   *mem;
    uint32_t    pc;
    uint32_t    sp;
    uint32_t    tos;
    uint32_t    nos;
    uint8_t     opcode;
    uint32_t    cpu;
    bool        pc_dirty;
    bool        decode_mask;
} zpu_t;

#define zpu_set_sp(zpu,v)       ((zpu)->sp = (v))
#define zpu_get_sp(zpu)         ((zpu)->sp)
#define zpu_inc_sp(zpu)         ((zpu)->sp+=4)
#define zpu_dec_sp(zpu)         ((zpu)->sp-=4)

#define zpu_set_pc(zpu,v)       ((zpu)->pc = (v))
#define zpu_get_pc(zpu)         ((zpu)->pc)

#define zpu_set_tos(zpu,v)      ((zpu)->tos = (v))
#define zpu_get_tos(zpu)        ((zpu)->tos)

#define zpu_set_nos(zpu,v)      ((zpu)->nos = (v))
#define zpu_get_nos(zpu)        ((zpu)->nos)

#define zpu_set_cpu(zpu,v)      ((zpu)->cpu = (v))
#define zpu_get_cpu(zpu)        ((zpu)->cpu)

#define zpu_set_mem(zpu,m)      ((zpu)->mem = (m))
#define zpu_get_mem(zpu)        ((zpu)->mem)

#define zpu_set_reset_sp(zpu,v) ((zpu)->reset_sp = (v))
#define zpu_get_reset_sp(zpu)   ((zpu)->reset_sp)

extern void zpu_reset   (zpu_t* zpu,uint32_t sp);
extern void zpu_execute (zpu_t* zpu);

/** consumer callbacks (weak bindings) */
extern void zpu_breakpoint_handler     (zpu_t* zpu);
extern void zpu_divzero_handler        (zpu_t* zpu);
extern void zpu_config_handler         (zpu_t* zpu);
extern void zpu_illegal_opcode_handler (zpu_t* zpu);

#endif // ZPU_H
