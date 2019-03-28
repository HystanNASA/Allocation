#include <stdio.h>

#include "allocation.h"

int main(int argc, char const *argv[])
{
    int* a = mmalloc(sizeof(int));
    int* b = ccalloc(2, sizeof(int));
    if(!a || !b)
    {
        printf("Doesn't work properly\n");
        return 1;
    }

    *a = 10;
    *b = 3;
    b[1] = 2;

    printf("%d\n", *a);
    printf("%d  %d\n", *b, b[1]);

    a = rrealloc(a, sizeof(int) * 2);

    a[1] = 5;
    printf("%d  %d\n", *a, *(a + 1));

    ffree(a);
    ffree(b);

    return 0;
}
