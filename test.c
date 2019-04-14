#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

#include "allocation.h"

int main(int argc, char const *argv[])
{    
    int* a = (int*)mmalloc(sizeof(int) * 530);
	int* b = (int*)mmalloc(sizeof(int) * 530);

	*a = 100;
	*b = 2000;

	printf("A: %d\n", *a);
	printf("B: %d\n", *b);

    ffree(a);
	ffree(b);

    return 0;
}