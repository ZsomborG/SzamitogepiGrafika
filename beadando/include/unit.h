#ifndef UNIT_H
#define UNIT_H

#include <cglm/cglm.h>
#include <stdbool.h>
#include <glad/glad.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "game_state.h"



struct Scene;
struct App;

typedef enum UnitType {
    UNIT_MELEE_TANK = 0,
    UNIT_RANGED_ARCHER,
    NUM_UNIT_TYPES
} UnitType;

typedef enum UnitLocation {
    LOC_NONE,
    LOC_BENCH,
    LOC_BOARD
} UnitLocation;

typedef enum UnitState {
    UNIT_STATE_IDLE,      // Default state
    UNIT_STATE_MOVING,    // (For later)
    UNIT_STATE_ATTACKING, // When attack cooldown is met
    UNIT_STATE_DEAD       // (For later)
} UnitState;

// --- Unit Data Structure ---
typedef struct Unit {
    int id;
    UnitType type;
    bool is_player_unit;
    UnitLocation location;
    
    // Position
    int grid_x;
    int grid_y;
    vec3 world_pos;
    
    // Stats
    float max_hp;
    float current_hp;
    float attack_damage;
    float attack_speed;
    float attack_range;
    
    // Movement/Targeting
    float aggro_range;
    vec3  facing_direction;
    float movement_speed;
    bool needs_to_move_for_attack;
    float move_cooldown_timer;
    
    // Combat
    UnitState current_combat_state;    // Tracks what the unit is doing
    struct Unit* current_target_ptr; // Pointer to the unit being targeted
    float attack_cooldown_timer;     // Time until next attack is ready
    bool is_attacking_visual_active; // Flag for the "attack" animation
    float attack_visual_timer;       // Timer for how long the visual stays active
    bool is_alive;
} Unit;

/**
 * @brief Initializes a unit instance with basic properties.
 * Calculates initial world_pos from grid coordinates.
 * @param unit Pointer to the Unit struct to initialize.
 * @param type The UnitType for this unit.
 * @param grid_x Initial grid x-coordinate.
 * @param grid_y Initial grid y-coordinate.
 * @param is_player True if it's a player unit, false for AI.
 * @param initial_location Initial location of the unit.
 */
void init_unit(Unit* unit, UnitType type, int grid_x, int grid_y, bool is_player, UnitLocation initial_location);

/**
 * @brief Renders a single unit.
 * Assumes the correct shader program is already in use.
 * Assumes View and Projection uniforms are set.
 * Needs access to Scene to get type-specific resources (VAO, texture).
 * @param unit Pointer to the unit to render.
 * @param scene Pointer to the main scene containing unit resources.
 * @param shader_program ID of the shader program.
 * @param model_uloc Uniform location for the 'model' matrix.
 */
void render_unit(const Unit* unit, const struct Scene* scene, GLuint shader_program, const struct App* app);

/**
 * @brief Updates a unit's state (e.g., animation, movement - for later).
 * @param unit Pointer to the unit to update.
 * @param dt Delta time since last update.
 */
void update_unit(Unit* unit, struct Scene* scene, float dt, GamePhase current_phase);

/**
 * @brief Gets the display name for a given UnitType.
 */
const char* get_unit_type_name(UnitType type);

/**
 * @brief Gets the gold cost for a given UnitType.
 */
int get_unit_cost(UnitType type);

bool is_tile_walkable(const struct Scene* scene, int grid_x, int grid_y, const Unit* moving_unit);

bool try_move_step(struct Scene* scene, Unit* unit, int target_grid_x, int target_grid_y);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* UNIT_H */