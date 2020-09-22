/*
 * ZPU
 *
 * A Virtual Machine for the ZPU architecture as defined by ZyLin Inc.
 *
 * By Michael Rychlik
 *
 * Based on an original idea by Toby Seckshund.
 *
 * Optimizations by Bill Henning.
 *
 * Due to the way the VM is optimized here, in order to minimize ZPU memory access,
 * it is rather hard to follow this code from the ZPU zpu->instruction set documentation.
 * Basically the variable "tos" is used to hold the value that is "top of stack" such
 * that it can be accessed quickly without pop/push pairs in many op codes.
 *
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include <zpu.h>
#include <zpu_load.h>
#include <zpu_memory.h>
#include <zpu_syscall.h>

static void     push(zpu_t* zpu,uint32_t data);
static uint32_t pop(zpu_t* zpu);
static void     printRegs(zpu_t* zpu);
static uint32_t flip(uint32_t i);


void emulate(zpu_t* zpu)
{
    push(zpu,zpu_get_tos(zpu));
    zpu_set_tos(zpu,zpu_get_pc(zpu) + 1);
    zpu_set_pc(zpu,(memoryReadByte(zpu_get_pc(zpu)) - 32) * VECTORSIZE + VECTORBASE);
    zpu->touchedPc = true;
}


void zpu_reset(zpu_t* zpu)
{
    memoryInitialize();
    sysinitialize();
    zpu_load();
    zpu_set_pc( zpu, 0 );
    zpu->instruction = 0;
    zpu->touchedPc   = true;
    zpu->decodeMask  = 0;
#if 0
    zpu_set_sp( zpu, 0xFFFFFFF );
#else
    zpu_set_sp( zpu, 0x1fff8 );
#endif
    zpu_set_tos( zpu, 0 );
}


void zpu_execute(zpu_t* zpu)
{
    for (;;)
    {
        zpu->touchedPc = false;

        zpu->instruction = memoryReadByte(zpu_get_pc(zpu));

        // printf( "%d\n",zpu->instruction);

#if 0
        static int step = 0;
        if (step == 0)
        {
            printf ("#pc,opcode,sp,top_of_stack,next_on_stack\n");
            printf ("#----------\n");
            printf ("\n");
        }
        printf ("0x%07x 0x%02x 0x%08x 0x%08x 0x%08x\n", zpu->pc, zpu->instruction, zpu->sp, zpu->tos, memoryReadLong(zpu->sp + 4));
        fflush(0);      
        //memoryDisplayLong(sp - 16*4, 32);
        //printf("\n");
        //getchar();
        if (step++ == 10000)
        {
            printf("Done.\n");
            fflush(0);  
            exit(0);
        }
#endif

        if ((zpu->instruction & 0x80) == ZPU_IM)
        {
            if (zpu->decodeMask)
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
            zpu->decodeMask = true;
        }
        else
        {
            zpu->decodeMask = false;
            if ((zpu->instruction & 0xF0) == ZPU_ADDSP)
            {
                uint32_t addr;
                addr = zpu->instruction & 0x0F;
                addr = zpu_get_sp(zpu) + addr * 4;
		        // Handle case were addr is sp. This DOES happen.
                if (addr == zpu_get_sp(zpu))
                    zpu_set_tos(zpu,zpu_get_tos(zpu) + zpu_get_tos(zpu));
                else
                    zpu_set_tos(zpu,zpu_get_tos(zpu) + memoryReadLong(addr));
            }
            else if ((zpu->instruction & 0xE0) == ZPU_LOADSP)
            {
                uint32_t addr;
		        //memoryWriteLong(sp, tos);                    // Need this?
                addr = (zpu->instruction & 0x1F) ^ 0x10;
                addr = zpu_get_sp(zpu) + 4 * addr;
                push(zpu,zpu_get_tos(zpu));
                zpu_set_tos(zpu,memoryReadLong(addr));
            }
            else if ((zpu->instruction & 0xE0) == ZPU_STORESP)
            {
                uint32_t addr;
                addr = (zpu->instruction & 0x1F) ^ 0x10;
                addr = zpu_get_sp(zpu) + 4 * addr;
                memoryWriteLong(addr, zpu_get_tos(zpu));
                zpu_set_tos(zpu,pop(zpu));
            }
            else
            {
                switch (zpu->instruction)
                {
                    case 0:
                        printf ("\nBreakpoint\n");
                        printRegs(zpu);
                        exit(1);
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
		                //memoryWriteLong(sp, tos);            // Need this?
                        zpu_set_tos(zpu, memoryReadLong(zpu_get_tos(zpu)) );
                        break;
                    case ZPU_PUSHSPADD:
                        zpu_set_tos(zpu,(zpu_get_tos(zpu) * 4) + zpu_get_sp(zpu));
                        break;
                    case ZPU_STORE:
                        zpu_set_nos(zpu,pop(zpu));
			            memoryWriteLong(zpu_get_tos(zpu), zpu_get_nos(zpu));
                        zpu_set_tos(zpu,pop(zpu));
                        break;
                    case ZPU_POPPC:
                        zpu_set_pc(zpu,zpu_get_tos(zpu));
                        zpu_set_tos(zpu,pop(zpu));
                        zpu->touchedPc = true;
                        break;
                    case ZPU_POPPCREL:
                        zpu_set_pc( zpu,zpu_get_pc(zpu) + zpu_get_tos(zpu) );
                        zpu_set_tos( zpu,pop(zpu) );
                        zpu->touchedPc = true;
                        break;
                    case ZPU_FLIP:
                        zpu_set_tos( zpu, flip( zpu_get_tos(zpu) ) );
                        break;
                    case ZPU_ADD:
                        zpu_set_nos( zpu, pop(zpu) );
                        zpu_set_tos( zpu,  zpu_get_tos(zpu) + zpu_get_nos(zpu) );
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
                        //memoryWriteLong(sp, tos);            // Need this ?
                        zpu_set_sp( zpu, zpu_get_tos(zpu) );
                        zpu_set_tos( zpu, memoryReadLong(zpu_get_sp(zpu)) );
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
		                //memoryWriteLong(sp, tos);            //Need this ?
                        zpu_set_tos( zpu, memoryReadByte(zpu_get_tos(zpu)) );
                        break;
                    case ZPU_STOREB:
                        zpu_set_nos( zpu, pop( zpu ) );
                        memoryWriteByte(zpu_get_tos(zpu),zpu_get_nos(zpu));
                        zpu_set_tos( zpu, pop(zpu) );
                        break;
                    case ZPU_LOADH:
		                //memoryWriteLong(sp, tos);            //Need this ?
                        zpu_set_tos( zpu, memoryReadWord(zpu_get_tos(zpu)) );
                        break;
                    case ZPU_STOREH:
                        zpu_set_nos( zpu, pop(zpu) );
                        memoryWriteWord(zpu_get_tos(zpu),zpu_get_nos(zpu));
                        zpu_set_tos( zpu, pop(zpu) );
                        break;
                    case ZPU_LESSTHAN:
                        zpu_set_nos( zpu, pop(zpu) );
                        if ((int32_t)zpu_get_tos(zpu) < (int32_t)zpu_get_nos(zpu))
                            zpu_set_tos( zpu, 1 );
                        else
                            zpu_set_tos( zpu, 0 );
                        break;
                    case ZPU_LESSTHANOREQUAL:
                        zpu_set_nos( zpu, pop(zpu) );
                        if ((int32_t)zpu_get_tos(zpu) <= (int32_t)zpu_get_nos(zpu))
                            zpu_set_tos( zpu, 1 );
                        else
                            zpu_set_tos( zpu, 0 );
                        break;
                    case ZPU_ULESSTHAN:
                        zpu_set_nos( zpu, pop(zpu) );
                        if (zpu_get_tos(zpu) < (int32_t)zpu_get_nos(zpu))
                            zpu_set_tos( zpu, 1 );
                        else
                            zpu_set_tos( zpu, 0 );
                        break;
                    case ZPU_ULESSTHANOREQUAL:
                        zpu_set_nos( zpu, pop(zpu) );
                        if (zpu_get_tos(zpu) <= (int32_t)zpu_get_nos(zpu))
                            zpu_set_tos( zpu, 1 );
                        else
                            zpu_set_tos( zpu, 0 );
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
                            zpu->touchedPc = true;
                        }
                        zpu_set_tos( zpu, pop(zpu) );
                        break;
                    case ZPU_NEQBRANCH:
                        zpu_set_nos( zpu, pop(zpu) );
                        if (zpu_get_nos(zpu) != 0)
                        {
                            zpu_set_pc(zpu,zpu_get_pc(zpu) + zpu_get_tos(zpu));
                            zpu->touchedPc = true;
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
                        {
                            printf("Divide by zero\n");
                            fflush(0);
                            exit(1);
                        }
                        zpu_set_tos( zpu, (int32_t)zpu_get_tos(zpu) / (int32_t)zpu_get_nos(zpu) );
                        break;
                    case ZPU_MOD:
                        zpu_set_nos( zpu, pop(zpu) );
                        if (zpu_get_nos(zpu) == 0)
                        {
                            printf("Divide by zero\n");
                            fflush(0);
                            exit(1);
                        }
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
                        zpu_set_tos( zpu, (int32_t)zpu_get_nos(zpu) >> (zpu_get_tos(zpu) & 0x3f) );
                        break;
                    case ZPU_CALL:
                        zpu_set_nos( zpu, zpu_get_tos(zpu) );
                        zpu_set_tos( zpu, zpu_get_pc(zpu) + 1 );
                        zpu_set_pc( zpu, zpu_get_nos(zpu) );
                        zpu->touchedPc = true;
                        break;
                    case ZPU_CALLPCREL:
                        zpu_set_nos( zpu, zpu_get_tos(zpu) );
                        zpu_set_tos( zpu, zpu_get_pc(zpu) + 1 );
                        zpu_set_pc( zpu, zpu_get_pc(zpu) + zpu_get_nos(zpu) );
                        zpu->touchedPc = true;
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
                        printf ("CONFIG indicates CPU type is %d\n", zpu_get_cpu(zpu));
                        break;
                    case ZPU_SYSCALL:
                        // Flush tos to real stack
                        memoryWriteLong(zpu_get_sp(zpu), zpu_get_tos(zpu));
                        syscall(zpu_get_sp(zpu));
                        break;
                    default:
                        printf ("Illegal Instruction\n");
                        printRegs(zpu);
                        exit(1);
                        break;
                }
            }
        }
        if (!zpu->touchedPc)
        {
            zpu_set_pc(zpu,zpu_get_pc(zpu) + 1);
            zpu->touchedPc = true;
        }
    }
}




static uint32_t pop(zpu_t* zpu)
{
    zpu_inc_sp(zpu);
    return memoryReadLong(zpu_get_sp(zpu));
}


static void push(zpu_t* zpu,uint32_t data)
{
    memoryWriteLong(zpu_get_sp(zpu), data);
    xpu_dec_sp(zpu);
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


static void printRegs(zpu_t* zpu)
{
    printf ("PC=%08x SP=%08x TOS=%08x OP=%02x DM=%02x debug=%08x\n", zpu_get_pc(zpu), zpu_get_sp(zpu), zpu_get_tos(zpu), zpu->instruction, zpu->decodeMask, 0);
    fflush(0);
}

