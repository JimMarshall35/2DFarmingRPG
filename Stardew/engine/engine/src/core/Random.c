#include "Random.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "AssertLib.h"

float Ra_FloatBetween(float min, float max)
{
    EASSERT(max > min); 
    float random = ((float) rand()) / (float) RAND_MAX;

    float range = max - min;  
    return (random*range) + min;
}

unsigned int Ra_SeedFromTime()
{
    unsigned int t = (unsigned int)time(NULL);
    srand(t);
    return t;
}

unsigned int Ra_RandZeroTo(int maxExclusive)
{
    return rand() % maxExclusive;
}