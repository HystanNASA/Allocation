#include <stdio.h>

#include "allocation.h"

int main(int argc, char const *argv[])
{
    int* a = mmalloc(sizeof(int));
    if(a == NULL)
    {
        printf("Doesn't work properly\n");
        return 1;
    }

    *a = 10;

    printf("%d\n", *a);

    ffree(a);

    return 0;
}
