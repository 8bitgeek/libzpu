#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <zpu.h>

#define MEM_SEG_TEXT_BASE   0x00000
#define MEM_SEG_STACK_BASE  0x1F800

#define MEM_SEG_TEXT_SZ     (1024*32)
#define MEM_SEG_STACK_SZ    (1024*2)

uint32_t mem_seg_text[MEM_SEG_TEXT_SZ/4];
uint32_t mem_seg_stack[MEM_SEG_STACK_SZ/4];

zpu_mem_t zpu_mem_seg_text;
zpu_mem_t zpu_mem_seg_stack;
zpu_t zpu;

int main(int argc, char *argv[])
{
    memset(mem_seg_text,0,MEM_SEG_TEXT_SZ);
    memset(mem_seg_stack,0,MEM_SEG_STACK_SZ);
    
    zpu_mem_init( (zpu_mem_t*)NULL, 
                  &zpu_mem_seg_text, 
                  "text", 
                  mem_seg_text, 
                  MEM_SEG_TEXT_BASE, 
                  MEM_SEG_TEXT_SZ );

    zpu_mem_init( &zpu_mem_seg_text,
                  &zpu_mem_seg_stack,
                  "stack",
                  mem_seg_stack,
                  MEM_SEG_STACK_BASE,
                  MEM_SEG_STACK_SZ);

    zpu_set_mem(&zpu,&zpu_mem_seg_text);
    
    zpu_reset(&zpu);
    zpu_execute(&zpu);
}
