#include "MathUtils.h"

float MathUtils::Clamp(float a, float upperBound, float lowerBound)
{
    float clampedValue = Min(upperBound, Max(a, lowerBound));
    return clampedValue;
}

float MathUtils::Min(float a, float b)
{
    float minValue = 0;
    
    // Find the smaller of the two values - if equal, it doesn't matter so return a
    if (a > b)
        minValue = b;
    else
        minValue = a;

    return minValue;
}

float MathUtils::Max(float a, float b)
{
    float maxValue = 0;

    // Find the smaller of the two values - if equal, it doesn't matter so return a
    if (a > b)
        maxValue = a;
    else
        maxValue = b;

    return maxValue;
}
