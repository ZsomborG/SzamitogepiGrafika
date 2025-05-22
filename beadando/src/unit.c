#include "unit.h"
#include "board.h" // For grid_to_world_pos
#include "scene.h" // For accessing scene resources (VAO, Texture)
#include "app.h"

#include <stdio.h> // For debug prints
#include <float.h>
#include <math.h>

#define ATTACK_VISUAL_DURATION 0.15f

void init_unit(Unit* unit, UnitType type, int grid_x, int grid_y, bool is_player, UnitLocation initial_location) {
    if (!unit) return;

    static int next_id = 0;
    unit->id = next_id++;
    unit->type = type;
    unit->is_player_unit = is_player;
    unit->location = initial_location; // Set location
    

    // Grid/World pos only really valid if on board
    unit->grid_x = grid_x;
    unit->grid_y = grid_y;
    if (initial_location == LOC_BOARD) {
        grid_to_world_pos(grid_x, grid_y, unit->world_pos);
        unit->world_pos[1] = 0.1f; // Slight Y offset
    } else {
        // Set world pos to something invalid or off-screen if on bench/none
        glm_vec3_copy(GLM_VEC3_ZERO, unit->world_pos); // Example: Set to origin
    }

    // --- Set initial stats based on type ---
    switch (type) {
        case UNIT_MELEE_TANK:
            unit->max_hp = 100.0f;
            unit->current_hp = 100.0f;
            unit->attack_damage = 10.0f;
            unit->attack_speed = 0.8f; // Slower
            unit->attack_range = BOARD_TILE_SIZE * 1.1f; // Slightly more than one tile
            break;
        case UNIT_RANGED_ARCHER:
            unit->max_hp = 60.0f;
            unit->current_hp = 60.0f;
            unit->attack_damage = 15.0f;
            unit->attack_speed = 1.1f; // Faster
            unit->attack_range = BOARD_TILE_SIZE * 3.5f; // 3-4 tiles range
            break;
            // Add cases for other unit types...
        default:
            // Default stats for unknown type
            unit->max_hp = 50.0f;
            unit->current_hp = 50.0f;
            unit->attack_damage = 5.0f;
            unit->attack_speed = 1.0f;
            unit->attack_range = BOARD_TILE_SIZE * 1.1f;
            break;
    }

    unit->current_hp = unit->max_hp; // Start with full HP

    // --- Combat fields ---
    unit->current_combat_state = UNIT_STATE_IDLE;
    unit->current_target_ptr = NULL;
    unit->attack_cooldown_timer = 0.0f;
    unit->is_attacking_visual_active = false;
    unit->attack_visual_timer = 0.0f;
    unit->is_alive = true; // Unit starts alive

    unit->aggro_range = unit->attack_range * 2.5f; // Example: aggro is 2.5x attack range
    if (type == UNIT_RANGED_ARCHER) { // Archers might have same aggro as attack range or slightly more
        unit->aggro_range = unit->attack_range * 1.2f;
    }
    // Default facing direction (e.g., towards opponent side if Y is depth)
    if (is_player) {
        glm_vec3_copy((vec3){0.0f, 0.0f, 1.0f}, unit->facing_direction); // Player units face positive Z
    } else {
        glm_vec3_copy((vec3){0.0f, 0.0f, -1.0f}, unit->facing_direction); // AI units face negative Z
    }
    unit->movement_speed = 0.75f; // Tiles per second (approx, due to discrete steps)
    unit->needs_to_move_for_attack = false;
    unit->move_cooldown_timer = 0.0f; // Can move immediately if needed
    
    printf("DEBUG: init_unit - Unit Type %d (ID %d) created at (%d, %d), Location: %d, Alive: %s\n",
           type, unit->id, grid_x, grid_y, initial_location, unit->is_alive ? "Yes" : "No");
}


void render_unit(const Unit* unit, const struct Scene* scene, GLuint shader_program, const struct App* app) {
    if (!unit || !scene || !app || !unit->is_alive || unit->location != LOC_BOARD || scene->unit_vaos[unit->type] == 0) {
        return;
    }

    mat4 model_matrix;
    glm_mat4_identity(model_matrix);

    // --- Make a non-const copy for glm_translate ---
    vec3 temp_world_pos;
    glm_vec3_copy(unit->world_pos, temp_world_pos);
    glm_translate(model_matrix, temp_world_pos); // Pass the non-const copy

    // --- Make a non-const copy for glm_vec3_norm2 ---
    vec3 temp_facing_direction;
    glm_vec3_copy(unit->facing_direction, temp_facing_direction);
    if (glm_vec3_norm2(temp_facing_direction) > 0.001f) {
        float yaw_angle_rad = atan2f(temp_facing_direction[0], temp_facing_direction[2]); // Use temp copy
        glm_rotate_y(model_matrix, yaw_angle_rad, model_matrix);
    }

    mat4 scale_matrix;
    glm_mat4_identity(scale_matrix);
    float current_display_scale = 0.8f; // Base display scale for units
    if (unit->is_attacking_visual_active) {
        current_display_scale *= 1.20f;
    }
    glm_scale(scale_matrix, (vec3){current_display_scale, current_display_scale, current_display_scale});
    glm_mat4_mul(model_matrix, scale_matrix, model_matrix); // Apply final scale


    GLint model_uloc = app->shader_uloc_model;
    if (model_uloc != -1) {
        glUniformMatrix4fv(model_uloc, 1, GL_FALSE, (const GLfloat*)model_matrix);
    }

    if (app->shader_uloc_materialDiffuse != -1) glUniform3fv(app->shader_uloc_materialDiffuse, 1, scene->material.diffuse);
    if (app->shader_uloc_materialSpecular != -1) glUniform3fv(app->shader_uloc_materialSpecular, 1, scene->material.specular);
    if (app->shader_uloc_materialShininess != -1) glUniform1f(app->shader_uloc_materialShininess, scene->material.shininess);
    check_gl_error("Set unit material uniforms");

    glBindTexture(GL_TEXTURE_2D, scene->unit_textures[unit->type]);
    glBindVertexArray(scene->unit_vaos[unit->type]);
    GLsizei index_count = scene->unit_index_counts[unit->type];
    if (index_count > 0) {
        glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, NULL);
    }
    glBindVertexArray(0);
}


void update_unit(Unit* unit, struct Scene* scene, float dt, GamePhase current_phase) {
    if (!unit || !scene) return;

    // 1. Handle non-participating units or non-combat phase
    if (!unit->is_alive || unit->location != LOC_BOARD) {
        unit->current_target_ptr = NULL;
        unit->current_combat_state = UNIT_STATE_IDLE;
        unit->is_attacking_visual_active = false;
        unit->attack_cooldown_timer = 0.0f;
        unit->move_cooldown_timer = 0.0f;
        unit->needs_to_move_for_attack = false;
        return;
    }

    if (current_phase != PHASE_COMBAT) {
        unit->current_target_ptr = NULL;
        unit->attack_cooldown_timer = 0.0f;
        unit->move_cooldown_timer = 0.0f;
        unit->is_attacking_visual_active = false;
        unit->current_combat_state = UNIT_STATE_IDLE;
        unit->needs_to_move_for_attack = false;
        if (unit->is_player_unit) glm_vec3_copy((vec3){0.0f, 0.0f, 1.0f}, unit->facing_direction);
        else glm_vec3_copy((vec3){0.0f, 0.0f, -1.0f}, unit->facing_direction);
        return;
    }

    // --- COMBAT PHASE LOGIC ---

    // 2. Update Timers
    if (unit->is_attacking_visual_active) {
        unit->attack_visual_timer -= dt;
        if (unit->attack_visual_timer <= 0.0f) {
            unit->is_attacking_visual_active = false;
        }
    }
    // Cooldowns are decremented *before* checking if an action can be performed
    if (unit->attack_cooldown_timer > 0.0f) unit->attack_cooldown_timer -= dt;
    if (unit->move_cooldown_timer > 0.0f) unit->move_cooldown_timer -= dt;


    // 3. TARGETING LOGIC (Validation, Acquisition, Switching)
    Unit* new_target_candidate = NULL; // Will hold the best target found this frame

    // 3.1 Validate current target
    if (unit->current_target_ptr != NULL) {
        bool current_target_still_valid = unit->current_target_ptr->is_alive &&
                                          unit->current_target_ptr->location == LOC_BOARD;
        if (unit->type == UNIT_RANGED_ARCHER && current_target_still_valid) {
            vec3 dist_vec;
            glm_vec3_sub(unit->current_target_ptr->world_pos, unit->world_pos, dist_vec);
            if (glm_vec3_norm2(dist_vec) > (unit->aggro_range * unit->aggro_range)) {
                // printf("DEBUG: Ranged Unit %d (ID %d) current target %d (ID %d) moved OUTSIDE aggro. Will re-evaluate.\n", unit->type, unit->id, unit->current_target_ptr->type, unit->current_target_ptr->id);
                current_target_still_valid = false; // Ranged unit loses target if it leaves aggro
            }
        }
        if (!current_target_still_valid) {
            unit->current_target_ptr = NULL; // Clear invalid target
        }
    }

    // 3.2 Find best potential target (closest in aggro, or closest overall)
    Unit* closest_enemy_in_aggro = NULL;
    Unit* closest_enemy_overall = NULL;
    float closest_enemy_in_aggro_dist_sq = FLT_MAX;
    float closest_enemy_overall_dist_sq = FLT_MAX;

    for (int i = 0; i < scene->unit_count; ++i) {
        Unit* other_unit = &scene->units[i];
        if (other_unit->id == unit->id || other_unit->is_player_unit == unit->is_player_unit ||
            !other_unit->is_alive || other_unit->location != LOC_BOARD) {
            continue;
        }
        vec3 dist_vec;
        glm_vec3_sub(other_unit->world_pos, unit->world_pos, dist_vec);
        float dist_sq = glm_vec3_norm2(dist_vec);

        if (dist_sq < closest_enemy_overall_dist_sq) {
            closest_enemy_overall_dist_sq = dist_sq;
            closest_enemy_overall = other_unit;
        }
        if (dist_sq <= (unit->aggro_range * unit->aggro_range)) {
            if (dist_sq < closest_enemy_in_aggro_dist_sq) {
                closest_enemy_in_aggro_dist_sq = dist_sq;
                closest_enemy_in_aggro = other_unit;
            }
        }
    }

    // 3.3 Check if being attacked by someone who should take priority
    Unit* current_attacker_priority = NULL;
    if (unit->is_player_unit) { // Player units might not auto-switch if already engaged
        // For now, player units stick to their target unless it dies
    } else { // AI units will switch to an attacker if it's a better/more immediate threat
        float closest_attacker_dist_sq_local = FLT_MAX;
        for (int i = 0; i < scene->unit_count; ++i) {
            Unit* other_unit = &scene->units[i];
            if (other_unit->is_player_unit != unit->is_player_unit &&
                other_unit->is_alive && other_unit->location == LOC_BOARD &&
                other_unit->current_target_ptr == unit) { // Is this other unit targeting ME?
                vec3 dist_vec;
                glm_vec3_sub(other_unit->world_pos, unit->world_pos, dist_vec);
                float dist_sq = glm_vec3_norm2(dist_vec);
                if (dist_sq < closest_attacker_dist_sq_local && dist_sq <= (unit->aggro_range * unit->aggro_range)) {
                    closest_attacker_dist_sq_local = dist_sq;
                    current_attacker_priority = other_unit;
                }
            }
        }
    }

    // 3.4 Assign new_target_candidate
    if (current_attacker_priority && current_attacker_priority != unit->current_target_ptr) {
        new_target_candidate = current_attacker_priority;
        printf("DEBUG: Unit %d (ID %d) SWITCHING TARGET to attacker %d (ID %d).\n", unit->type, unit->id, new_target_candidate->type, new_target_candidate->id);
    } else if (unit->current_target_ptr == NULL && closest_enemy_in_aggro != NULL) {
        new_target_candidate = closest_enemy_in_aggro;
        printf("DEBUG: Unit %d (ID %d) acquired NEW TARGET (closest in aggro) -> Unit %d (ID %d)\n", unit->type, unit->id, new_target_candidate->type, new_target_candidate->id);
    }
    // If new_target_candidate is set, update the unit's actual target
    if (new_target_candidate) {
        unit->current_target_ptr = new_target_candidate;
    }


    // 4. DETERMINE CURRENT COMBAT STATE AND MOVEMENT INTENT
    unit->needs_to_move_for_attack = false; // Reset

    if (unit->current_target_ptr != NULL) { // Has a specific engagement target
        glm_vec3_sub(unit->current_target_ptr->world_pos, unit->world_pos, unit->facing_direction);
        if(glm_vec3_norm2(unit->facing_direction) > 0.001f) glm_vec3_normalize(unit->facing_direction);

        float dist_to_target_sq = glm_vec3_distance2(unit->world_pos, unit->current_target_ptr->world_pos);

        if (dist_to_target_sq <= (unit->attack_range * unit->attack_range)) {
            unit->current_combat_state = UNIT_STATE_ATTACKING;
        } else { // Out of attack range, target is in aggro
            if (unit->type == UNIT_MELEE_TANK) {
                unit->current_combat_state = UNIT_STATE_MOVING;
                unit->needs_to_move_for_attack = true;
            } else if (unit->type == UNIT_RANGED_ARCHER) {
                unit->current_combat_state = UNIT_STATE_MOVING;
                unit->needs_to_move_for_attack = true;
            }
        }
    } else { // No specific engagement target
        if (closest_enemy_overall != NULL) { // Enemies exist, advance
            unit->current_combat_state = UNIT_STATE_MOVING;
            glm_vec3_sub(closest_enemy_overall->world_pos, unit->world_pos, unit->facing_direction);
            if(glm_vec3_norm2(unit->facing_direction) > 0.001f) glm_vec3_normalize(unit->facing_direction);
        } else {
            unit->current_combat_state = UNIT_STATE_IDLE; // No enemies at all
        }
    }

    // 5. EXECUTE ACTIONS (Movement, then Attack)

    // Execute Movement
    if (unit->current_combat_state == UNIT_STATE_MOVING && unit->move_cooldown_timer <= 0.0f) {
        bool moved = false;
        Unit* actual_move_target = unit->needs_to_move_for_attack ? unit->current_target_ptr : closest_enemy_overall;

        if (actual_move_target) {
            // (Logging for movement intent)
            moved = try_move_step(scene, unit, actual_move_target->grid_x, actual_move_target->grid_y);
            if (moved) {
                grid_to_world_pos(unit->grid_x, unit->grid_y, unit->world_pos);
                unit->world_pos[1] = 0.1f;
                unit->move_cooldown_timer = 1.0f / unit->movement_speed;
                // printf("DEBUG: Unit %d (ID %d) moved to grid (%d, %d).\n", unit->type, unit->id, unit->grid_x, unit->grid_y);

                // If was moving to engage (melee), re-check if in attack range & update state
                if (unit->current_target_ptr && unit->needs_to_move_for_attack) {
                    float dist_sq_after_move = glm_vec3_distance2(unit->world_pos, unit->current_target_ptr->world_pos);
                    if (dist_sq_after_move <= (unit->attack_range * unit->attack_range)) {
                        unit->needs_to_move_for_attack = false;
                        unit->current_combat_state = UNIT_STATE_ATTACKING;
                    }
                }
            } else {
                unit->move_cooldown_timer = 0.25f; // Blocked
            }
        }
    }

    // Execute Attack
    if (unit->current_combat_state == UNIT_STATE_ATTACKING && unit->attack_cooldown_timer <= 0.0f) {
        if (unit->current_target_ptr != NULL && unit->current_target_ptr->is_alive) {
            // (Attack logic: print, visual, damage, check death, reset cooldown)
            // This is where your "AI ARCHER (ID %d) attacking target %d..." log would go if you added it
            // ...
            unit->is_attacking_visual_active = true;
            unit->attack_visual_timer = ATTACK_VISUAL_DURATION;
            unit->current_target_ptr->current_hp -= unit->attack_damage;
            printf("DEBUG: Unit %d (P%d, T%d) ATTACKS Unit %d (P%d, T%d) for %.1f dmg. Target HP: %.1f\n",
                   unit->id, unit->is_player_unit, unit->type,
                   unit->current_target_ptr->id, unit->current_target_ptr->is_player_unit, unit->current_target_ptr->type,
                   unit->attack_damage, unit->current_target_ptr->current_hp);

            if (unit->current_target_ptr->current_hp <= 0.0f) {
                unit->current_target_ptr->current_hp = 0.0f;
                unit->current_target_ptr->is_alive = false;
                printf("DEBUG: Unit %d (ID %d) has died!\n", unit->current_target_ptr->type, unit->current_target_ptr->id);
                unit->current_target_ptr = NULL;
                unit->current_combat_state = UNIT_STATE_IDLE; // Re-evaluate next frame
            }
            unit->attack_cooldown_timer = 1.0f / unit->attack_speed;
        } else { // Target became invalid just before attack
            unit->current_target_ptr = NULL;
            unit->current_combat_state = UNIT_STATE_IDLE;
        }
    }
}

// --- Unit Information Functions ---
const char* get_unit_type_name(UnitType type) { // Renamed from GetUnitTypeName for consistency
    switch(type) {
        case UNIT_MELEE_TANK: return "Tank";
        case UNIT_RANGED_ARCHER: return "Archer";
            // Add other types as they are defined
        default: return "Unknown Unit";
    }
}

int get_unit_cost(UnitType type) {
    switch(type) {
        case UNIT_MELEE_TANK: return 3; // Example cost
        case UNIT_RANGED_ARCHER: return 3; // Example cost
            // Add costs for other types
        default: return 99; // Cost for unknown/default
    }
}

bool is_tile_walkable(const struct Scene* scene, int grid_x, int grid_y, const Unit* moving_unit) {
    if (!scene || !moving_unit) return false;

    // Check board bounds
    if (grid_x < 0 || grid_x >= BOARD_GRID_WIDTH || grid_y < 0 || grid_y >= BOARD_GRID_HEIGHT) {
        return false; // Off board
    }

    // Check if occupied by any other LIVE unit on the BOARD
    for (int i = 0; i < scene->unit_count; ++i) {
        const Unit* other_unit = &scene->units[i];
        if (other_unit->id == moving_unit->id) continue; // Skip self

        if (other_unit->is_alive && other_unit->location == LOC_BOARD &&
            other_unit->grid_x == grid_x && other_unit->grid_y == grid_y) {
            return false; // Occupied
        }
    }
    return true; // Walkable
}

bool try_move_step(struct Scene* scene, Unit* unit, int target_grid_x, int target_grid_y) {
    if (!scene || !unit) return false;

    int current_grid_x = unit->grid_x;
    int current_grid_y = unit->grid_y;

    int dx = 0;
    if (target_grid_x > current_grid_x) dx = 1;
    else if (target_grid_x < current_grid_x) dx = -1;

    int dy_grid = 0;
    if (target_grid_y > current_grid_y) dy_grid = 1;
    else if (target_grid_y < current_grid_y) dy_grid = -1;

    if (dx == 0 && dy_grid == 0) return false; // Already at target grid cell

    // Try diagonal first, then cardinal
    if (dx != 0 && dy_grid != 0) {
        if (is_tile_walkable(scene, current_grid_x + dx, current_grid_y + dy_grid, unit)) {
            unit->grid_x += dx;
            unit->grid_y += dy_grid;
            return true;
        }
    }
    // Try X-axis move
    if (dx != 0) {
        if (is_tile_walkable(scene, current_grid_x + dx, current_grid_y, unit)) {
            unit->grid_x += dx;
            return true;
        }
    }
    // Try Y-axis move
    if (dy_grid != 0) {
        if (is_tile_walkable(scene, current_grid_x, current_grid_y + dy_grid, unit)) {
            unit->grid_y += dy_grid;
            return true;
        }
    }
    return false; // No valid move found this step
}