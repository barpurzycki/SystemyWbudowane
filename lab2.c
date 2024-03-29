#include <stdio.h>
#include <stdlib.h>

#define BIT_VALUE(value, noBit) (value >> noBit) & 1
unsigned int counter = 1;

void delay()
{
    int a, b;
    for(a = 0; a <= 10000000; a++)
    {
        for(b = 0; b <= 512; b++)
        {

        }
    }
}

unsigned int generateMLS()
{
    unsigned int bit0 = BIT_VALUE(counter, 0);
    unsigned int bit1 = BIT_VALUE(counter, 1);
    unsigned int bit4 = BIT_VALUE(counter, 4);
    unsigned int bit5 = BIT_VALUE(counter, 5);

    unsigned int result = ((bit0 ^ bit1) ^ bit4) ^ bit5;

    return result;
}

int main()
{
    while(1)
    {
        printf("\r");
        printf("%d", generateMLS());
        for(int i = 0; i < 5; i++)
        {
            printf("%d", BIT_VALUE(counter, i));
        }
        counter = (counter >> 1) | (generateMLS() << 5);
        delay();
    }
    return 0;
}
