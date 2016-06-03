#ifndef TINYPE_TOOL_H
#define TINYPE_TOOL_H

#include <stdlib.h>  
#include <time.h>

static unsigned long long myseed = 1;  // 只是供随机数生成器使用

void srand48(unsigned int i);

double drand48(void);


#endif