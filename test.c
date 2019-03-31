#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

#include "allocation.h"

int main(int argc, char const *argv[])
{    
    int* a = (int*)mmalloc(sizeof(int));
    int* b = (int*)mmalloc(sizeof(int) * 2);

    if(!a)
    {
        printf("Doesn't work\n");
        return 1;
    }

    ffree(a);
    ffree(b);

    return 0;
}