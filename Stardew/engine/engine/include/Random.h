#ifndef RANDOM_H
#define RANDOM_H

float Ra_FloatBetween(float min, float max);
unsigned int Ra_SeedFromTime();
unsigned int Ra_RandZeroTo(int maxExclusive);

#endif