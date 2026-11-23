#include "FloatingPointLib.h"

bool CompareFloat(float f1, float f2)
{
    float precision = 0.00001;
    if (((f1 - precision) < f2) &&
        ((f1 + precision) > f2))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
