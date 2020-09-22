#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "zpu.h"

zpu_t zpu;

// Print usage instructions
void usage()
{
    printf ("ZOG v0.11\n");
}


int main(int argc, char *argv[])
{
    usage();
    zpu_reset(&zpu);
    zpu_execute(&zpu);
}
