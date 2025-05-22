#ifndef SCENE_H
#define SCENE_H

#include "camera.h"
#include "texture.h"
#include "board.h"
#include <obj/model.h>
#include "utils.h"
#include "unit.h"
#include "board.h"

#define MAX_UNITS 50
#define BENCH_SIZE 8

struct App;

typedef struct Scene
{
    Board board;
    Material material;
    
    Model unit_models[NUM_UNIT_TYPES];
    GLuint unit_textures[NUM_UNIT_TYPES];
    GLuint unit_vaos[NUM_UNIT_TYPES];
    
    GLsizei unit_index_counts[NUM_UNIT_TYPES];
    
    Unit units[MAX_UNITS];
    int unit_count;
} Scene;

/**
 * Initialize the scene by loading models.
 */
void init_scene(Scene* scene);

/**
 * Set the lighting of the scene.
 */
void set_lighting();

/**
 * Set the current material.
 */
void set_material(const Material* material);

/**
 * Update the scene.
 */
void update_scene(Scene* scene, float dt, GamePhase current_phase);

/**
 * Render the scene objects.
 */
struct InputState;
void render_scene(const Scene* scene, GLuint shader_program, const struct App* app, int selected_bench_unit_index);

/**
 * Draw the origin of the world coordinate system.
 */
void draw_origin();

/**
 * Frees resources associated with the scene (GPU buffers, textures).
 */
 void destroy_scene(Scene* scene);

// --- Add New Function Declaration ---
/**
 * @brief Attempts to add a new unit of the specified type to the bench.
 * Finds an inactive slot in the scene->units array.
 * @param scene Pointer to the Scene.
 * @param type The UnitType to add.
 * @return Pointer to the newly added Unit if successful (and bench not full), NULL otherwise.
 */
Unit* add_unit_to_bench(Scene* scene, UnitType type);

#endif /* SCENE_H */
