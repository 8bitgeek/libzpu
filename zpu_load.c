#include <stdio.h>

#include "zpu_load.h"
#include "zpu_memory.h"


void zpu_load()
{
        char* fileName = "test.bin";
	FILE* f;
	int bytesRead;
	int address;
	uint8_t inByte;

	f = fopen(fileName, "r");
	if (f == 0)
	{
		printf("Failed to open %s\n", fileName);
		perror("");
		exit(0);
	}
	for (address = 0; address < memorySize(); address++)
	{
		bytesRead = fread(&inByte, 1, 1, f);
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
		memoryWriteByte(address, inByte);		
	}
	//printf("Loaded %d bytes from RAM image from %s\n", address, fileName);
	fclose(f);
}

