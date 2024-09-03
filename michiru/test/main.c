#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char** argv)
{
	int i;
	for(i = 0; i < argc; i++)
		printf("arg%d: %s\n", i, argv[i]);
    	printf("exiting\n");
	return 0;
}
