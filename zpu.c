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
#include <stdbool.h>
#include <stdlib.h>

#include <zpu.h>
#include <zpu_mem.h>
#include <zpu_syscall.h>

static void     push(zpu_t* zpu,uint32_t data);
static uint32_t pop(zpu_t* zpu);
static void     printRegs(zpu_t* zpu);
static uint32_t flip(uint32_t i);



void zpu_reset(zpu_t* zpu,uint32_t sp)
{
    zpu_set_sp  ( zpu, sp );
    zpu_set_pc  ( zpu, 0 );
    zpu_set_tos ( zpu, 0 );
    zpu->instruction = 0;
    zpu->pc_dirty    = true;
    zpu->decode_mask = 0;
}

void zpu_execute(zpu_t* zpu)
{
    for (;;)
    {
        zpu->pc_dirty = false;

        zpu->instruction = zpu_mem_get_opcode( zpu_get_mem(zpu), zpu_get_pc(zpu) );

        if ((zpu->instruction & 0x80) == ZPU_IM)
        {
            if (zpu->decode_mask)
            {
                zpu_set_tos(zpu,zpu_get_tos(zpu) << 7);
                zpu_set_tos(zpu, zpu_get_tos(zpu) | (zpu->instruction & 0x7f) );
            }
            else
            {
                push(zpu,zpu_get_tos(zpu));
                zpu_set_tos(zpu,zpu->instruction << 25);
                zpu_set_tos(zpu,((int32_t)zpu_get_tos(zpu)) >> 25);
            }
            zpu->decode_mask = true;
        }
        else
        {
            zpu->decode_mask = false;
            if ((zpu->instruction & 0xF0) == ZPU_ADDSP)
            {
                uint32_t addr;
                addr = zpu->instruction & 0x0F;
                addr = zpu_get_sp(zpu) + addr * 4;
		        // Handle case were addr is sp.
                if (addr == zpu_get_sp(zpu))
                    zpu_set_tos(zpu,zpu_get_tos(zpu) + zpu_get_tos(zpu));
                else
                    zpu_set_tos(zpu,zpu_get_tos(zpu) + zpu_mem_get_uint32( zpu_get_mem(zpu), addr));
            }
            else if ((zpu->instruction & 0xE0) == ZPU_LOADSP)
            {
                uint32_t addr;
                addr = (zpu->instruction & 0x1F) ^ 0x10;
                addr = zpu_get_sp(zpu) + 4 * addr;
                push(zpu,zpu_get_tos(zpu));
                zpu_set_tos(zpu,zpu_mem_get_uint32( zpu_get_mem(zpu), addr));
            }
            else if ((zpu->instruction & 0xE0) == ZPU_STORESP)
            {
                uint32_t addr;
                addr = (zpu->instruction & 0x1F) ^ 0x10;
                addr = zpu_get_sp(zpu) + 4 * addr;
                zpu_mem_set_uint32( zpu_get_mem(zpu), addr, zpu_get_tos(zpu));
                zpu_set_tos(zpu,pop(zpu));
            }
            else
            {
                switch (zpu->instruction)
                {
                    case 0:
                        zpu_breakpoint_handler(zpu);
                        break;
                    case ZPU_PUSHPC:
                        push(zpu,zpu_get_tos(zpu));
                        zpu_set_tos(zpu,zpu_get_pc(zpu));
                        break;
                    case ZPU_OR:
                        zpu_set_nos(zpu,pop(zpu));
                        zpu_set_tos(zpu, zpu_get_tos(zpu) | zpu_get_nos(zpu) );
                        break;
                    case ZPU_NOT:
                        zpu_set_tos(zpu, ~zpu_get_tos(zpu) );
                        break;
                    case ZPU_LOAD:
                        zpu_set_tos(zpu, zpu_mem_get_uint32( zpu_get_mem(zpu), zpu_get_tos(zpu)) );
                        break;
                    case ZPU_PUSHSPADD:
                        zpu_set_tos(zpu,(zpu_get_tos(zpu) * 4) + zpu_get_sp(zpu));
                        break;
                    case ZPU_STORE:
                        zpu_set_nos(zpu,pop(zpu));
			            zpu_mem_set_uint32( zpu_get_mem(zpu), zpu_get_tos(zpu), zpu_get_nos(zpu));
                        zpu_set_tos(zpu,pop(zpu));
                        break;
                    case ZPU_POPPC:
                        zpu_set_pc(zpu,zpu_get_tos(zpu));
                        zpu_set_tos(zpu,pop(zpu));
                        zpu->pc_dirty = true;
                        break;
                    case ZPU_POPPCREL:
                        zpu_set_pc( zpu,zpu_get_pc(zpu) + zpu_get_tos(zpu) );
                        zpu_set_tos( zpu,pop(zpu) );
                        zpu->pc_dirty = true;
                        break;
                    case ZPU_FLIP:
                        zpu_set_tos( zpu, flip( zpu_get_tos(zpu) ) );
                        break;
                    case ZPU_ADD:
                        zpu_set_nos( zpu, pop(zpu) );
                        zpu_set_tos( zpu, zpu_get_tos(zpu) + zpu_get_nos(zpu) );
                        break;
                    case ZPU_SUB:
                        zpu_set_nos( zpu, pop( zpu ) );
                        zpu_set_tos( zpu, zpu_get_nos(zpu) - zpu_get_tos(zpu) );
                        break;
                    case ZPU_PUSHSP:
    			        push( zpu, zpu_get_tos(zpu) );
			            zpu_set_tos( zpu, zpu_get_sp(zpu) + 4 );
                        break;
                    case ZPU_POPSP:
                        zpu_set_sp( zpu, zpu_get_tos(zpu) );
                        zpu_set_tos( zpu, zpu_mem_get_uint32( zpu_get_mem(zpu), zpu_get_sp(zpu)) );
                        break;
                    case ZPU_NOP:
                        break;
                    case ZPU_AND:
                        zpu_set_nos( zpu, pop( zpu ) );
                        zpu_set_tos( zpu, zpu_get_tos(zpu) & zpu_get_nos(zpu) );
                        break;
                    case ZPU_XOR:
                        zpu_set_nos( zpu, pop( zpu ) );
                        zpu_set_tos( zpu, zpu_get_tos(zpu) ^ zpu_get_nos(zpu) );
                        break;
                    case ZPU_LOADB:
                        zpu_set_tos( zpu,zpu_mem_get_uint8( zpu_get_mem(zpu), zpu_get_tos(zpu)) );
                        break;
                    case ZPU_STOREB:
                        zpu_set_nos( zpu, pop( zpu ) );
                        zpu_mem_set_uint8( zpu_get_mem(zpu), zpu_get_tos(zpu),zpu_get_nos(zpu));
                        zpu_set_tos( zpu, pop(zpu) );
                        break;
                    case ZPU_LOADH:
                        zpu_set_tos( zpu, zpu_mem_get_uint16( zpu_get_mem(zpu), zpu_get_tos(zpu)) );
                        break;
                    case ZPU_STOREH:
                        zpu_set_nos( zpu, pop(zpu) );
                        zpu_mem_set_uint16( zpu_get_mem(zpu), zpu_get_tos(zpu),zpu_get_nos(zpu));
                        zpu_set_tos( zpu, pop(zpu) );
                        break;
                    case ZPU_LESSTHAN:
                        zpu_set_nos( zpu, pop(zpu) );
                        zpu_set_tos( zpu, ((int32_t)zpu_get_tos(zpu) < (int32_t)zpu_get_nos(zpu)) ? 1 : 0 );
                        break;
                    case ZPU_LESSTHANOREQUAL:
                        zpu_set_nos( zpu, pop(zpu) );
                        zpu_set_tos( zpu, ((int32_t)zpu_get_tos(zpu) <= (int32_t)zpu_get_nos(zpu)) ? 1 :0 );
                        break;
                    case ZPU_ULESSTHAN:
                        zpu_set_nos( zpu, pop(zpu) );
                        zpu_set_tos( zpu, (zpu_get_tos(zpu) < (int32_t)zpu_get_nos(zpu)) ? 1 : 0 );
                        break;
                    case ZPU_ULESSTHANOREQUAL:
                        zpu_set_nos( zpu, pop(zpu) );
                        zpu_set_tos( zpu, (zpu_get_tos(zpu) <= (int32_t)zpu_get_nos(zpu)) ? 1 : 0 );
                        break;
                    case ZPU_SWAP:
                        zpu_set_tos( zpu, ((zpu_get_tos(zpu) >> 16) & 0xffff) | (zpu_get_tos(zpu) << 16) );
                        break;
                    case ZPU_MULT16X16:
                        zpu_set_nos( zpu, pop(zpu) );
                        zpu_set_tos( zpu, ((int32_t)zpu_get_nos(zpu) & 0xffff) * (zpu_get_tos(zpu) & 0xffff) );
                        break;
                    case ZPU_EQBRANCH:
                        zpu_set_nos( zpu, pop(zpu) );
                        if ((int32_t)zpu_get_nos(zpu) == 0)
                        {
                            zpu_set_pc(zpu,zpu_get_pc(zpu) + zpu_get_tos(zpu));
                            zpu->pc_dirty = true;
                        }
                        zpu_set_tos( zpu, pop(zpu) );
                        break;
                    case ZPU_NEQBRANCH:
                        zpu_set_nos( zpu, pop(zpu) );
                        if (zpu_get_nos(zpu) != 0)
                        {
                            zpu_set_pc(zpu,zpu_get_pc(zpu) + zpu_get_tos(zpu));
                            zpu->pc_dirty = true;
                        }
                        zpu_set_tos( zpu, pop(zpu) );
                        break;
                    case ZPU_MULT:
                        zpu_set_nos( zpu, pop(zpu) );
                        zpu_set_tos( zpu, (int32_t)zpu_get_tos(zpu) * (int32_t)zpu_get_nos(zpu) );
                        break;
                    case ZPU_DIV:
                        zpu_set_nos( zpu, pop(zpu) );
                        if (zpu_get_nos(zpu) == 0)
                            zpu_divzero_handler(zpu);
                        zpu_set_tos( zpu, (int32_t)zpu_get_tos(zpu) / (int32_t)zpu_get_nos(zpu) );
                        break;
                    case ZPU_MOD:
                        zpu_set_nos( zpu, pop(zpu) );
                        if (zpu_get_nos(zpu) == 0)
                            zpu_divzero_handler(zpu);
                        zpu_set_tos( zpu, (int32_t)zpu_get_tos(zpu) % (int32_t)zpu_get_nos(zpu) );
                        break;
                    case ZPU_LSHIFTRIGHT:
                        zpu_set_nos( zpu, pop(zpu) );
                        zpu_set_tos( zpu, zpu_get_nos(zpu) >> (zpu_get_tos(zpu) & 0x3f) );
                        break;
                    case ZPU_ASHIFTLEFT:
                        zpu_set_nos( zpu, pop(zpu) );
                        zpu_set_tos( zpu, zpu_get_nos(zpu) << (zpu_get_tos(zpu) & 0x3f) );
                        break;
                    case ZPU_ASHIFTRIGHT:
                        zpu_set_nos( zpu, pop(zpu) );
                        zpu_set_tos( zpu, zpu_get_nos(zpu) >> (zpu_get_tos(zpu) & 0x3f) );
                        break;
                    case ZPU_CALL:
                        zpu_set_nos( zpu, zpu_get_tos(zpu) );
                        zpu_set_tos( zpu, zpu_get_pc(zpu) + 1 );
                        zpu_set_pc( zpu, zpu_get_nos(zpu) );
                        zpu->pc_dirty = true;
                        break;
                    case ZPU_CALLPCREL:
                        zpu_set_nos( zpu, zpu_get_tos(zpu) );
                        zpu_set_tos( zpu, zpu_get_pc(zpu) + 1 );
                        zpu_set_pc( zpu, zpu_get_pc(zpu) + zpu_get_nos(zpu) );
                        zpu->pc_dirty = true;
                        break;
                    case ZPU_EQ:
                        zpu_set_nos( zpu, pop(zpu) );
                        if (zpu_get_nos(zpu) == zpu_get_tos(zpu))
                            zpu_set_tos( zpu, 1 );
                        else
                            zpu_set_tos( zpu, 0 );
                        break;
                    case ZPU_NEQ:
                        zpu_set_nos( zpu, pop(zpu) );
                        if (zpu_get_nos(zpu) != zpu_get_tos(zpu))
                            zpu_set_tos( zpu, 1 );
                        else
                            zpu_set_tos( zpu, 0 );
                        break;
                    case ZPU_NEG:
                        zpu_set_tos( zpu, -zpu_get_tos(zpu) );
                        break;
                    case ZPU_CONFIG:
                        zpu_set_cpu(zpu,zpu_get_tos(zpu));
                        zpu_set_tos( zpu, pop(zpu) );
                        zpu_config_handler(zpu);
                        break;
                    case ZPU_SYSCALL:
                        zpu_mem_set_uint32( zpu_get_mem(zpu), zpu_get_sp(zpu), zpu_get_tos(zpu));
                        zpu_syscall(zpu);
                        break;
                    default:
                        zpu_illegal_opcode_handler(zpu);
                        break;
                }
            }
        }
        if (!zpu->pc_dirty)
        {
            zpu_set_pc(zpu,zpu_get_pc(zpu) + 1);
            zpu->pc_dirty = true;
        }
    }
}

static uint32_t pop(zpu_t* zpu)
{
    zpu_inc_sp(zpu);
    return zpu_mem_get_uint32( zpu_get_mem(zpu), zpu_get_sp(zpu));
}


static void push(zpu_t* zpu,uint32_t data)
{
    zpu_mem_set_uint32( zpu_get_mem(zpu), zpu_get_sp(zpu), data);
    zpu_dec_sp(zpu);
}

static uint32_t flip(uint32_t i)
{
    uint32_t t = 0;
    for (int j = 0; j < 32; j++)
    {
        t |= ((i >> j) & 1) << (31 - j);
    }
    return t;
}

void zpu_emulate(zpu_t* zpu)
{
    push( zpu, zpu_get_tos(zpu) );
    zpu_set_tos( zpu, zpu_get_pc(zpu) + 1 );
    zpu_set_pc( zpu, ( zpu_mem_get_uint8( zpu_get_mem(zpu), zpu_get_pc(zpu) ) - 32) * VECTORSIZE + VECTORBASE);
    zpu->pc_dirty = true;
}

void __attribute__((weak)) zpu_breakpoint_handler(zpu_t* zpu) 
{
    /* NOP*/
}

void __attribute__((weak)) zpu_divzero_handler(zpu_t* zpu)
{
    /* NOP */
}

void __attribute__((weak)) zpu_config_handler(zpu_t* zpu)
{
    /* NOP */
}

void __attribute__((weak)) zpu_illegal_opcode_handler(zpu_t* zpu)
{
    /* NOP */
}

