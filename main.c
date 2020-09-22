#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <zpu.h>

#define MEMORY_SIZE (1024*1024)
uint32_t text_mem[MEMORY_SIZE/4];

zpu_mem_t zpu_mem;
zpu_t zpu;

int main(int argc, char *argv[])
{
    memset(text_mem,0,MEMORY_SIZE);
    zpu_mem_init((zpu_mem_t*)NULL,&zpu_mem,"text",text_mem,0,MEMORY_SIZE);
    zpu_set_mem(&zpu,&zpu_mem);
    zpu_reset(&zpu);
    zpu_execute(&zpu);
}
