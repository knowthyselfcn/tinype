#include "tool.h"

void srand48(unsigned int i)
{
    myseed = (((long long int)i) << 16) | rand();
}

double drand48(void)
{
    srand48(time(NULL));

    myseed = (0x5DEECE66DLL * myseed + 0xB16) & 0xFFFFFFFFFFFFLL;
    unsigned int x = myseed >> 16;
    return  ((double)x / (double)0x100000000LL);
}
