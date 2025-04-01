#ifndef UTILS_H
#define UTILS_H

#include <math.h>

/**
 * GLSL-like three dimensional vector
 */
typedef struct vec3
{
    float x;
    float y;
    float z;
} vec3;

/**
 * Color with RGB components
 */
typedef struct Color
{
    float red;
    float green;
    float blue;
} Color;

/**
 * Calculates radian from degree.
 */
double degree_to_radian(double degree);

vec3 vec3_add(vec3 a, vec3 b);

vec3 vec3_subtract(vec3 a, vec3 b);

vec3 vec3_scale(vec3 v, float s);

float vec3_dot(vec3 a, vec3 b);

vec3 vec3_cross(vec3 a, vec3 b);

float vec3_length(vec3 v);

vec3 vec3_normalize(vec3 v);

#endif /* UTILS_H */
