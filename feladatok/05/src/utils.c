#include "utils.h"

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

double degree_to_radian(double degree)
{
	return degree * M_PI / 180.0;
}

vec3 vec3_add(vec3 a, vec3 b)
{
	vec3 result = { a.x + b.x, a.y + b.y, a.z + b.z };
	return result;
}

vec3 vec3_sub(vec3 a, vec3 b)
{
	vec3 result = { a.x - b.x, a.y - b.y, a.z - b.z };
	return result;
}

vec3 vec3_scale(vec3 v, float s)
{
	vec3 result = { v.x * s, v.y * s, v.z * s };
	return result;
}

float vec3_dot(vec3 a, vec3 b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

vec3 vec3_cross(vec3 a, vec3 b)
{
	vec3 result = { 
		a.y * b.z - a.z * b.y, 
		a.z * b.x - a.x * b.z, 
		a.x * b.y - a.y * b.x 
	};
	return result;
}

float vec3_length(vec3 v)
{
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

vec3 vec3_normalize(vec3 v)
{
	float len = vec3_length(v);
	vec3 result = { 0.0f, 0.0f, 0.0f };
	if (len > 1e-6)
	{
		float inv_len = 1.0f / len;
		result.x = v.x * inv_len;
		result.y = v.y * inv_len;	
		result.z = v.z * inv_len;
	}
	return result;
}