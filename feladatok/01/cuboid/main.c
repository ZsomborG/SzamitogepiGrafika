#include "cuboid.h"

#include <stdio.h>
#include <stdbool.h>

int main(int argc, char* argv[])
{
	Cuboid cuboid;
	double volume;
    double surface;
    bool cube;
	
	set_size(&cuboid, 5, 10, 10);

	volume = calc_volume(&cuboid);
    surface = calc_surface(&cuboid);
	
	printf("Cuboid volume: %lf\n", volume);
    printf("Cuboid surface: %lf\n", surface);

    cube = check_cube(&cuboid);

    if(cube == true)
    {
        printf("Van a teglatestnek negyzet alaku lapja");
    }
    else
    {
        printf("Nincs a teglatestnek negyzet alaku lapja");
    }
	
	return 0;
}
