#ifndef CUBOID_H
#define CUBOID_H

#include <stdbool.h>

typedef struct Cuboid
{
	double x;
	double y;
    double z;
} Cuboid;

void set_size(Cuboid* cuboid, double x, double y, double z);

double calc_volume(const Cuboid* cuboid);

double calc_surface(const Cuboid* cuboid);

bool check_cube(const Cuboid* cuboid);

#endif // CUBOID_H
