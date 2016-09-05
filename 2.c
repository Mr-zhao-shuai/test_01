#include <stdio.h>

int main(void)
{
    int month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int year = 0;
    int mon = 0;
    int day = 0;

    printf("请输入年份\n");
    scanf("%d", &year);
    printf("请输入天数\n");
    scanf("%d", &day);

    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
        month[1] = 29;
    while (day > *(month + mon++)) {
        day -= *(month + mon - 1);
    }
    printf("%d月%d天\n", mon, day);

    return 0;
}
