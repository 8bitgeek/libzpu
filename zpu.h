
#ifndef ZPU_H
#define ZPU_H

#include <stdint.h>
#include <stdbool.h>
#include <zpu_mem.h>

#define ZPU_IM               128
#define ZPU_BREAKPOINT       0
#define ZPU_PUSHSP           2
#define ZPU_POPPC            4
#define ZPU_ADD              5
#define ZPU_AND              6
#define ZPU_OR               7
#define ZPU_LOAD             8
#define ZPU_NOT              9
#define ZPU_FLIP             10
#define ZPU_NOP              11
#define ZPU_STORE            12
#define ZPU_POPSP            13
#define ZPU_ADDSP            16
#define ZPU_EMULATE          32
#define ZPU_LOADH            34
#define ZPU_STOREH           35
#define ZPU_LESSTHAN         36
#define ZPU_LESSTHANOREQUAL  37
#define ZPU_ULESSTHAN        38
#define ZPU_ULESSTHANOREQUAL 39
#define ZPU_SWAP             40
#define ZPU_MULT             41
#define ZPU_LSHIFTRIGHT      42
#define ZPU_ASHIFTLEFT       43
#define ZPU_ASHIFTRIGHT      44
#define ZPU_CALL             45
#define ZPU_EQ               46
#define ZPU_NEQ              47
#define ZPU_NEG              48
#define ZPU_SUB              49
#define ZPU_XOR              50
#define ZPU_LOADB            51
#define ZPU_STOREB           52
#define ZPU_DIV              53
#define ZPU_MOD              54
#define ZPU_EQBRANCH         55
#define ZPU_NEQBRANCH        56
#define ZPU_POPPCREL         57
#define ZPU_CONFIG           58
#define ZPU_PUSHPC           59
#define ZPU_SYSCALL          60
#define ZPU_PUSHSPADD        61
#define ZPU_MULT16X16        62
#define ZPU_CALLPCREL        63
#define ZPU_STORESP          64
#define ZPU_LOADSP           96

#define VECTORSIZE           0x20
#define VECTOR_RESET         0
#define VECTOR_INTERRUPT     1
#define VECTORBASE           0x0

typedef struct _zpu_
{
    zpu_mem_t   *mem;
    uint32_t    pc;
    uint32_t    sp;
    uint32_t    tos;
    uint32_t    nos;
    uint8_t     instruction;
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

void zpu_reset(zpu_t* zpu);
void zpu_execute(zpu_t* zpu);

#endif // ZPU_H
