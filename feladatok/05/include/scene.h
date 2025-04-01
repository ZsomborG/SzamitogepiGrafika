#ifndef SCENE_H
#define SCENE_H

#include "camera.h"
#include <GL/gl.h>

typedef struct Scene
{
    float sphere_rotation_angle;
} Scene;

/**
 * Initialize the scene by loading models.
 */
void init_scene(Scene* scene);

/**
 * Update the scene.
 */
void update_scene(Scene* scene, double time);

/**
 * Render the scene objects.
 */
void render_scene(const Scene* scene);

/**
 * Draw the origin of the world coordinate system.
 */
void draw_origin();

/**
 * Draw a tessellated sphere.
 */
void draw_sphere(float radius, int slices, int stacks);

/**
 * Draw a checkerboard pattern on the XY plane (Z=0).
 */
void draw_checkerboard(int width_squares, int height_squares, float square_size);

/**
 * Draw a cylinder approximation using triangle strip and fans.
 */
void draw_cylinder(float radius, float height, int segments);

/**
 * Draw a cone approximation using triangle fans.
 */
void draw_cone(float radius, float height, int segments);

#endif /* SCENE_H */
