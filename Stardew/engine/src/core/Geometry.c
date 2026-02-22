#include "Geometry.h"
#include <math.h>

bool Ge_PointInAABB(float pointX, float pointY, GeomRect rect)
{
    return pointX >= rect[GR_TLX] && pointX <= rect[GR_BRX] &&
        pointY >= rect[GR_TLY] && pointY <= rect[GR_BRY];
}



bool Ge_AABBIntersect(vec2 tl1, vec2 br1, vec2 tl2, vec2 br2)
{
    // Check for X-axis overlap
    bool xOverlap = br1[0] > tl2[0] && br2[0] > tl1[0];

    // Check for Y-axis overlap
    bool yOverlap = br1[1] > tl2[1] && br2[1] > tl1[1];

    // Collision occurs only if both axes overlap
    return xOverlap && yOverlap;
}


float Ge_AngleBetweenVec2s(vec2 a, vec2 b)
{
    float d = glm_vec2_dot(a, b);
    float ma = glm_vec2_norm(a);
    float mb = glm_vec2_norm(b);
    d = d / (ma * mb);
    return (float)acos(d);
}

float Ge_DistanceBetweenPoints(vec2 a, vec2 b)
{
    vec2 c;
    glm_vec2_sub(a, b, c);
    return fabs(glm_vec2_norm(c));
}