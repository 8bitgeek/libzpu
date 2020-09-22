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
