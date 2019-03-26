#include <stdio.h>

#include "allocation.h"

int main(int argc, char const *argv[])
{
    int* a = mmalloc(sizeof(int));

    *a = 10;

    printf("%d\n", *a);

    ffree(a);

    return 0;
}
