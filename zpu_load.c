#include <stdio.h>
#include <stdlib.h>

#include <zpu_load.h>
#include <zpu_mem.h>

void zpu_load(zpu_t* zpu)
{
    char* fileName = "test.bin";
	FILE* f;
	int address;
	uint8_t inByte;

	f = fopen(fileName, "r");
	if (f == 0)
	{
		printf("Failed to open %s\n", fileName);
		perror("");
		exit(0);
	}
	for (address = 0; address < zpu_mem_get_size(zpu_get_mem(zpu)); address++)
	{
		fread(&inByte, 1, 1, f);
		if (ferror(f))
		{
			printf("Error reading RAM image from %s\n", fileName);
			perror("");
			exit(1);
		}
		if (feof(f))
		{
			break;
		}
		zpu_mem_set_uint8( zpu_get_mem(zpu), address, inByte );		
	}
	//printf("Loaded %d bytes from RAM image from %s\n", address, fileName);
	fclose(f);
}

