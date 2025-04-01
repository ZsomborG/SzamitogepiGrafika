#ifndef CIRCLE_H
#define CIRCLE_H

#include <SDL2/SDL.h>
#include <math.h>

typedef struct {
    Uint8 r, g, b, a;
} Color;

typedef struct Circle
{
    double x;
    double y;
    double radius;
    Color color;
} Circle;

void set_circle_data(Circle* circle, double x, double y, double radius);

void set_circle_color(Circle* circle, Color color);

double calc_circle_area(const Circle* circle);


#endif