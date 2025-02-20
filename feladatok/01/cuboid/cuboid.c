#include "cuboid.h"

#include <math.h>
#include <stdbool.h>

void set_size(Cuboid* cuboid, double x, double y, double z)
{
	cuboid->x = x;
	cuboid->y = y;
    cuboid->z = z;
}

double calc_volume(const Cuboid* cuboid)
{
	double area = cuboid->x * cuboid->y * cuboid->z;
	return area;
}

double calc_surface(const Cuboid* cuboid)
{
    double surface = (2 * cuboid->x) + (2 * cuboid->y) + (2 * cuboid->z);
    return surface;
} 

bool check_cube(const Cuboid* cuboid)
{
    if(cuboid->x == cuboid->y)
    {
        return true;
    }
    else if(cuboid->x == cuboid->z)
    {
        return true;
    }
    else if(cuboid->y == cuboid->z)
    {
        return true;
    }
    else
    {
        return false;
    }
}