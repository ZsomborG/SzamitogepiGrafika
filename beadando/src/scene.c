#include "scene.h"
#include "utils.h" // For Material struct (will be replaced)

#include <glad/glad.h>
#include <cglm/cglm.h> // Include cglm if needed here

// These includes might need adjustment depending on draw_model implementation
#include <obj/load.h>
#include <obj/draw.h> // Needs implementation (using modern GL)
#include <obj/model.h>
#include "board.h"
#include "texture.h"
#include "unit.h"
#include "game_state.h"
#include "input.h"
#include "app.h"

#include <stdio.h>
#include <string.h>

// --- Add to Bench ---
Unit* add_unit_to_bench(Scene* scene, UnitType type) {
    if (!scene) return NULL;

    // Count how many units are currently on the bench
    int current_bench_count = 0;
    for (int i = 0; i < scene->unit_count; ++i) {
        if (scene->units[i].location == LOC_BENCH) {
            current_bench_count++;
        }
    }

    // Check if bench is full
    if (current_bench_count >= BENCH_SIZE) {
        printf("DEBUG: add_unit_to_bench - BENCH FULL (%d/%d)\n", current_bench_count, BENCH_SIZE);
        return NULL; // Bench is full
    }

    // Check if total unit capacity is reached
    if (scene->unit_count >= MAX_UNITS) {
        printf("DEBUG: add_unit_to_bench - MAX UNITS REACHED (%d/%d)\n", scene->unit_count, MAX_UNITS);
        return NULL; // No more space in the main array
    }

    // Find the next available slot in the main units array
    Unit* new_unit_slot = &scene->units[scene->unit_count];

    // Initialize the unit in that slot, placing it on the bench
    init_unit(new_unit_slot, type, -1, -1, true, LOC_BENCH); // Use invalid grid coords

    scene->unit_count++; // Increment total unit count

    printf("DEBUG: add_unit_to_bench - Added Unit Type %d to bench. Total Units: %d, Bench Count: %d\n",
           type, scene->unit_count, current_bench_count + 1);

    return new_unit_slot; // Return pointer to the new unit
}

// --- Add destroy_scene function ---
void destroy_scene(Scene* scene) {
    printf("DEBUG: destroy_scene - START\n");
    if (scene) {
        destroy_board(&scene->board);

        printf("DEBUG: destroy_scene - Destroying unit type resources...\n");
        for (int i = 0; i < NUM_UNIT_TYPES; ++i) {
            printf("DEBUG: destroy_scene - Destroying type %d\n", i);
            if (scene->unit_vaos[i] != 0)
            {
                destroy_model_buffers(&scene->unit_models[i]);
                scene->unit_vaos[i] = 0;
            }
            
            free_model(&scene->unit_models[i]);

            if (scene->unit_textures[i] != 0){
                glDeleteTextures(1, &scene->unit_textures[i]);
                scene->unit_textures[i] = 0;
            }
        }

        printf("DEBUG: destroy_scene - Unit resources destroyed.\n");
        
        scene->unit_count = 0;
    }
    printf("DEBUG: destroy_scene - END\n");
}

void init_scene(Scene* scene)
{
    printf("DEBUG: init_scene - START\n");
    if (!scene) {
        return;
    }

    // Material setup
    // Diffuse: How much it reflects light evenly. Often matches texture color.
    glm_vec3_copy((vec3){1.0f, 1.0f, 1.0f}, scene->material.diffuse);
    // Ambient: How much it reflects ambient light. Often a darker shade of diffuse.
    glm_vec3_copy((vec3){0.7f, 0.7f, 0.7f}, scene->material.ambient);
    // Specular: Color of the highlight. Often white or grey for non-metals.
    glm_vec3_copy((vec3){0.5f, 0.5f, 0.5f}, scene->material.specular);
    // Shininess: How focused the highlight is. Higher = sharper/smaller.
    scene->material.shininess = 32.0f;
    
    scene->unit_count = 0;
    
    // --- Initialize board ---
    if (!init_board(&scene->board, "assets/models/asd.obj", "assets/textures/grid.png")) {
        fprintf(stderr, "ERROR: init_scene - Failed to initialize board\n");
    }
    
    // --- Load Unit Type Resources ---
    printf("DEBUG: init_scene - Loading unit type resources...\n");
    
    // --- Filenames for types ---
    const char* model_files[NUM_UNIT_TYPES] = {
            [UNIT_MELEE_TANK] = "assets/models/up.obj",
            [UNIT_RANGED_ARCHER] = "assets/models/cube.obj"
    };
    
    const char* texture_files[NUM_UNIT_TYPES] = {
            [UNIT_MELEE_TANK] = "assets/textures/cube.png",
            [UNIT_RANGED_ARCHER] = "assets/textures/cube.png"
    };

    for (int i = 0; i < NUM_UNIT_TYPES; ++i) {
        printf("DEBUG: init_scene - Loading resources for unit type %d\n", i);
        
        scene->unit_vaos[i] = 0;
        scene->unit_textures[i] = 0;
        scene->unit_index_counts[i] = 0;
        init_model(&scene->unit_models[i]);

        // Load model
        if (!load_model(&scene->unit_models[i], model_files[i])){
            fprintf(stderr, "ERROR: init_scene - Failed to load model for unit type %d (%s)\n", i, model_files[i]);
            continue;
        }

        // Setup Buffers
        if (scene->unit_models[i].n_vertices > 0 && scene->unit_models[i].n_triangles > 0) {
            setup_model_buffers(&scene->unit_models[i]);
            if(scene->unit_models[i].vao_id == 0) {
                fprintf(stderr, "ERROR: init_scene - Failed to setup buffers for unit type %d\n", i);
                // free_model(&scene->unit_models[i]); // Free CPU data if buffers fail
                continue;
            }
            // Store VAO and index count for rendering
            scene->unit_vaos[i] = scene->unit_models[i].vao_id;
            scene->unit_index_counts[i] = scene->unit_models[i].index_count;
        } else {
            fprintf(stderr, "ERROR: init_scene - Model loaded for unit type %d has no vertices/triangles.\n", i);
            // free_model(&scene->unit_models[i]);
            continue;
        }
        
        // Load Texture
        scene->unit_textures[i] = load_texture(texture_files[i]);
        if (scene->unit_textures[i] == 0) {
            fprintf(stderr, "ERROR: init_scene - Failed to load texture for unit type %d (%s)\n", i, texture_files[i]);
            // Note: Model/buffers are loaded, maybe use a default texture? For now, just log error.
        }
        printf("DEBUG: init_scene - Resources loaded for type %d (VAO=%u, Tex=%u, Indices=%d)\n",
               i, scene->unit_vaos[i], scene->unit_textures[i], scene->unit_index_counts[i]);
    }
    printf("DEBUG: init_scene - Unit type resources loaded.\n");


    // --- Initialize Units ---
    // Change initial units to be placed directly on the board
    printf("DEBUG: init_scene - Creating initial board units...\n");
    if (scene->unit_count < MAX_UNITS) {
        // Use init_unit with LOC_BOARD
        init_unit(&scene->units[scene->unit_count++], UNIT_MELEE_TANK, 3, 1, true, LOC_BOARD);
    }
    if (scene->unit_count < MAX_UNITS) {
        init_unit(&scene->units[scene->unit_count++], UNIT_RANGED_ARCHER, 4, 1, true, LOC_BOARD);
    }
/*    if (scene->unit_count < MAX_UNITS) {
        init_unit(&scene->units[scene->unit_count++], UNIT_MELEE_TANK, 3, 6, false, LOC_BOARD); // AI unit
    }
    if (scene->unit_count < MAX_UNITS) {
        init_unit(&scene->units[scene->unit_count++], UNIT_RANGED_ARCHER, 4, 6, false, LOC_BOARD); // AI unit
    }*/
    printf("DEBUG: init_scene - %d initial board units created.\n", scene->unit_count);

    printf("DEBUG: init_scene - END\n");
}

// Update update_scene to pass current_phase
void update_scene(Scene* scene, float dt, GamePhase current_phase) // Added current_phase parameter
{
    if (!scene) return;
    for (int i = 0; i < scene->unit_count; ++i) {
        update_unit(&scene->units[i], scene, dt, current_phase); // Pass scene and current_phase
    }
}

void render_scene(const Scene* scene, GLuint shader_program, const App* app, int selected_bench_unit_index)
{
    if (!scene || !app) return;
    
    // Reset tint for normal objects
    if (app->shader_uloc_color_tint != -1) {
        glUniform4f(app->shader_uloc_color_tint, 1.0f, 1.0f, 1.0f, 1.0f);
        check_gl_error("Set default tint for scene (render_scene)");
    }

    render_board(&scene->board, shader_program, app);
    check_gl_error("after render_board (in render_scene)");

    glActiveTexture(GL_TEXTURE0); // Good practice
    for (int i = 0; i < scene->unit_count; ++i) {
        if (scene->units[i].location == LOC_BOARD && scene->units[i].is_alive) {
            // Reset tint before drawing each unit, as ghost might change it
            if (app->shader_uloc_color_tint != -1) {
                glUniform4f(app->shader_uloc_color_tint, 1.0f, 1.0f, 1.0f, 1.0f);
            }
            render_unit(&scene->units[i], scene, shader_program, app);
            check_gl_error("after render_unit in loop");
        }
    }

    // --- Render "Ghost" of Unit Being Placed ---
    if (selected_bench_unit_index != -1 && app->input_state.is_mouse_over_board) {
        if (selected_bench_unit_index >= 0 && selected_bench_unit_index < scene->unit_count) {
            const Unit* unit_to_preview = &scene->units[selected_bench_unit_index];
            if (unit_to_preview->location == LOC_BENCH) {
                UnitType preview_type = unit_to_preview->type;

                bool on_player_side = is_tile_on_player_side(app->input_state.hovered_grid_y);
                bool tile_empty = is_tile_empty_for_player(scene, app->input_state.hovered_grid_x, app->input_state.hovered_grid_y);

                if (scene->unit_vaos[preview_type] != 0) {
                    // --- Set Tint for Ghost ---
                    if (app->shader_uloc_color_tint != -1) {
                        if (on_player_side && tile_empty) {
                            glUniform4f(app->shader_uloc_color_tint, 0.7f, 1.0f, 0.7f, 0.65f); // Light green, semi-transparent
                        } else {
                            glUniform4f(app->shader_uloc_color_tint, 1.0f, 0.7f, 0.7f, 0.65f); // Light red, semi-transparent
                        }
                    }

                    mat4 ghost_model_matrix;
                    // ... (calculate ghost_model_matrix as before) ...
                    glm_mat4_identity(ghost_model_matrix);
                    vec3 ghost_world_pos;
                    grid_to_world_pos(app->input_state.hovered_grid_x, app->input_state.hovered_grid_y, ghost_world_pos);
                    ghost_world_pos[1] = 0.1f;
                    glm_translate(ghost_model_matrix, ghost_world_pos);
                    float base_scale = 0.8f;
                    glm_scale(ghost_model_matrix, (vec3){base_scale, base_scale, base_scale});


                    if (app->shader_uloc_model != -1) {
                        glUniformMatrix4fv(app->shader_uloc_model, 1, GL_FALSE, (const GLfloat*)ghost_model_matrix);
                        check_gl_error("Reset tint after ghost (render_scene end)");
                    }
                    glBindTexture(GL_TEXTURE_2D, scene->unit_textures[preview_type]);
                    glBindVertexArray(scene->unit_vaos[preview_type]);
                    GLsizei index_count = scene->unit_index_counts[preview_type];
                    if (index_count > 0) {
                        glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, NULL);
                    }
                    glBindVertexArray(0);

                    // --- Reset Tint after drawing ghost ---
                    if (app->shader_uloc_color_tint != -1) {
                        glUniform4f(app->shader_uloc_color_tint, 1.0f, 1.0f, 1.0f, 1.0f);
                    }
                }
            }
        }
    }
}