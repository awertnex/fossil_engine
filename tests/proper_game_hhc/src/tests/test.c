#include <stdio.h>
#include <math.h>

#include <raylib.h>

#include <engine/h/defines.h>

void test_binary_operators()
{
    u8 x = 63;
    printf("before: %d\n", x);
    x |= 0;
    printf("or 0: %d\n", x);
    x |= 0;
    printf("or 0: %d\n", x);
    x &= ~32;
    printf("nand 63: %d\n", x);
    x &= ~32;
    printf("nand 63: %d\n", x);
}

u8 parse = 1;
void test_gravity_vs_jump_height()
{
    f32 height = 1, pos = 0, fall = height;
    for (u8 i = 0; i < 100 && parse; ++i)
    {
        if (pos < 0)
        {
            pos = 0;
            parse = 0;
        }
        printf("height: %3.1f    pos: %4.2f    fall: %5.2f\n",
                height, pos, fall);

        fall -= (height/GRAVITY)/4;
        pos += fall/16;
    }
}

void test_state_checking_speed()
{
    u8 arr[400000000] = {0};
    arr[1999] = 49;
    arr[399999998] = 48;
    for (u32 o = 0; o < 400000000; ++o)
    {
        if (arr[o])
            printf("value: %c\n", arr[o]);
    }
}

void test_sin_cosin_rad2deg()
{
    for (f32 i = 0; i <= 90; ++i)
        //printf("angle: %3.0f\tsin: %6.5f\tcos: %6.5f\n",
        //i, sin(i/(PI*18.2378f)), cos(i/(PI*18.2378f)));

        printf("angle: %3.0f\tsin: %6.5f\tcos: %6.5f\n",
                i, sin(i/RAD2DEG), cos(i/RAD2DEG));
}
