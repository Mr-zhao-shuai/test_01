#include <stdio.h>

int main(void)
{
    int num[10] = {0};
    int *num_p = num;
    int all = 0;

    srand(time(0));
    while (all < 10) {
        num[all] = rand() % 100 + 1;
        all++;
    }

    for (all = 0; all < 10; all++) {
        printf("%d\n", *(num_p + all));
    }

    return 0;
}
