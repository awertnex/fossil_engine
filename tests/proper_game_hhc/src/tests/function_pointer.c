#include <stdio.h>

void button(void (*fp)())
{
    fp();
}

void f()
{
    printf("call successful\n");
}

int main(void)
{
    button(&f);
    return 0;
}
