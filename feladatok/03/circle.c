// circle.c
#include "circle.h" // Includes Color struct, Circle struct, function prototypes
#include <math.h>   // For isnan, M_PI (if not defined elsewhere)

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/*
 * NOTE: The color constant definitions (COLOR_RED, etc.) have been REMOVED
 * from this file. They are now defined directly in main_sdl.c where needed,
 * using compound literals like {r, g, b, a} or (Color){r, g, b, a}.
 * This resolves the "initializer element is not constant" error for global arrays
 * that tried to use these variables as initializers.
 */


void set_circle_data(Circle* circle, double x, double y, double radius)
{
    if (!circle) return; // Basic null check

    circle->x = x;
    circle->y = y;
    if (radius > 0.0) {
        circle->radius = radius;
    } else {
        // Indicate invalid radius using NAN (Not-a-Number)
        // Requires <math.h>
        circle->radius = NAN;
        // Alternative: Set a default small positive radius: circle->radius = 1.0;
        // Alternative: Return an error code.
    }
    // Initialize color to black by default when only setting data.
    // The caller can override this using set_circle_color.
    circle->color = (Color){0, 0, 0, 255};
}

void set_circle_color(Circle* circle, Color color)
{
    if (!circle) return; // Basic null check
    circle->color = color;
}


double calc_circle_area(const Circle* circle)
{
    if (!circle || isnan(circle->radius) || circle->radius <= 0.0) {
        // Return 0 or NAN for invalid circles
        return 0.0;
        // return NAN;
    }

    double area = circle->radius * circle->radius * M_PI;
    return area;
}