#include "app.h"

#include <glad/glad.h>
#include <cglm/cglm.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

#include "shader.h"
#include "scene.h"
#include "imgui_interface.h" // Include the C wrapper header
#include "board.h"
#include "input.h"
#include "game_state.h"
#include "unit.h"

#define ACTION_FLAG_NONE           0
#define ACTION_FLAG_REFRESH_SHOP   (1 << 0)
#define ACTION_FLAG_BUY_UNIT_TYPE_0     (1 << 1)
#define ACTION_FLAG_BUY_UNIT_TYPE_1     (1 << 2)
#define ACTION_FLAG_START_COMBAT   (1 << 10)

void cache_shader_uniform_locations(App* app) {
    if (app->shader_program == 0) {
        printf("[ERROR] cache_shader_uniform_locations: shader_program is 0!\n");
        return;
    }
    printf("DEBUG: cache_shader_uniform_locations - Using program %u\n", app->shader_program);
    glUseProgram(app->shader_program);
    check_gl_error("glUseProgram in cache_uniforms");

    app->shader_uloc_projection = glGetUniformLocation(app->shader_program, "projection");
    check_gl_error("glGetUniformLocation for projection");
    app->shader_uloc_view = glGetUniformLocation(app->shader_program, "view");
    check_gl_error("glGetUniformLocation for view");
    app->shader_uloc_model = glGetUniformLocation(app->shader_program, "model");
    check_gl_error("glGetUniformLocation for model");
    app->shader_uloc_texture1 = glGetUniformLocation(app->shader_program, "texture1");
    check_gl_error("glGetUniformLocation for texture1");
    app->shader_uloc_color_tint = glGetUniformLocation(app->shader_program, "uColorTint");
    check_gl_error("glGetUniformLocation for uColorTint");
    app->shader_uloc_lightDir = glGetUniformLocation(app->shader_program, "lightDir_world");
    check_gl_error("glGetUniformLocation for lightDir_world");
    app->shader_uloc_lightColor = glGetUniformLocation(app->shader_program, "lightColor");
    check_gl_error("glGetUniformLocation for lightColor");
    app->shader_uloc_ambientLightColor = glGetUniformLocation(app->shader_program, "ambientLightColor");
    check_gl_error("glGetUniformLocation for ambientLightColor");
    app->shader_uloc_viewPos = glGetUniformLocation(app->shader_program, "viewPos_world");
    check_gl_error("glGetUniformLocation for viewPos_world");
    app->shader_uloc_materialDiffuse = glGetUniformLocation(app->shader_program, "materialDiffuseColor");
    check_gl_error("glGetUniformLocation for materialDiffuseColor");
    app->shader_uloc_materialSpecular = glGetUniformLocation(app->shader_program, "materialSpecularColor");
    check_gl_error("glGetUniformLocation for materialSpecularColor");
    app->shader_uloc_materialShininess = glGetUniformLocation(app->shader_program, "materialShininess");
    check_gl_error("glGetUniformLocation for materialShininess");

    printf("[INFO] Cached shader lighting uniforms: LightDir=%d, LightColor=%d, Ambient=%d, ViewPos=%d, MatDiff=%d, MatSpec=%d, MatShine=%d\n",
           app->shader_uloc_lightDir, app->shader_uloc_lightColor, app->shader_uloc_ambientLightColor,
           app->shader_uloc_viewPos, app->shader_uloc_materialDiffuse, app->shader_uloc_materialSpecular,
           app->shader_uloc_materialShininess);

    printf("[INFO] Cached shader uniform locations: Proj=%d, View=%d, Model=%d, Tex1=%d, Tint=%d\n",
           app->shader_uloc_projection, app->shader_uloc_view, app->shader_uloc_model,
           app->shader_uloc_texture1, app->shader_uloc_color_tint);
    
    if (app->shader_uloc_color_tint != -1) {
        printf("DEBUG: cache_shader_uniform_locations - Attempting to set uColorTint (loc %d) immediately.\n", app->shader_uloc_color_tint);
        glUniform4f(app->shader_uloc_color_tint, 0.5f, 0.5f, 0.5f, 0.5f); // Use different values for test
        check_gl_error("TEST Set uColorTint immediately in cache_uniforms");
    } else {
        printf("[WARN] uColorTint uniform not found in shader program %u during cache!\n", app->shader_program);
    }

    glUseProgram(0);
    check_gl_error("glUseProgram(0) in cache_uniforms");
}

void init_app(App* app, int width, int height)
{
    printf("DEBUG: init_app - START\n");
    int error_code;
    int inited_loaders;

    app->is_running = false;
    app->uptime = 0.0; // Initialize uptime
    app->selected_bench_unit_index = -1;
    app->show_help_window = false;

    // --- Initialize Lighting Properties ---
    printf("DEBUG: init_app - Initializing Lighting...\n");
    // Directional light pointing from above-right-front towards origin
    glm_vec3_copy((vec3){0.5f, 1.0f, 0.7f}, app->light_direction_world); // Direction *towards* light source
    glm_vec3_normalize(app->light_direction_world); // Ensure it's normalized
    glm_vec3_copy((vec3){1.0f, 1.0f, 1.0f}, app->light_color);        // Bright white light
    glm_vec3_copy((vec3){0.25f, 0.25f, 0.3f}, app->ambient_light_color); // Dim ambient

    // --- Initialize input_state ---
    memset(&app->input_state, 0, sizeof(InputState));
    app->input_state.hovered_grid_x = -1;
    app->input_state.hovered_grid_y = -1;

    // --- SDL Init ---
    error_code = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS); // Init only necessary subsystems
    if (error_code != 0) {
        printf("[ERROR] SDL initialization error: %s\n", SDL_GetError());
        return;
    }

    // --- OpenGL Attributes ---
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); // Enable double buffering
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24); // Request depth buffer bits

    
    // --- SDL Window ---
    app->window = SDL_CreateWindow(
        "AutoChess Game (Phase 1)", // Updated Title
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE); // Add resizable flag
    if (app->window == NULL) {
        printf("[ERROR] Unable to create the application window: %s\n", SDL_GetError());
        SDL_Quit();
        return;
    }

    // --- SDL_Image ---
    inited_loaders = IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) & (IMG_INIT_PNG | IMG_INIT_JPG); // Init PNG and JPG support
    if ((inited_loaders & (IMG_INIT_PNG | IMG_INIT_JPG)) != (IMG_INIT_PNG | IMG_INIT_JPG)) {
        printf("[ERROR] IMG initialization error: %s\n", IMG_GetError());
        SDL_DestroyWindow(app->window);
        SDL_Quit();
        return;
    }
    
    // --- OpenGL ---
    app->gl_context = SDL_GL_CreateContext(app->window);
    if (app->gl_context == NULL) {
        printf("[ERROR] Unable to create the OpenGL context: %s\n", SDL_GetError());
        IMG_Quit(); // Quit SDL_image
        SDL_DestroyWindow(app->window);
        SDL_Quit();
        return;
    }


    // Print GL version
    //printf("[INFO] OpenGL Loaded\n");
    //printf("[INFO]   Vendor: %s\n", glGetString(GL_VENDOR));
    //printf("[INFO]   Renderer: %s\n", glGetString(GL_RENDERER));
    //printf("[INFO]   Version: %s\n", glGetString(GL_VERSION));
    //printf("[INFO]   Shading Language: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    // --- GLAD ---
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        printf("[ERROR] Failed to initialize GLAD\n");
        SDL_GL_DeleteContext(app->gl_context);
        IMG_Quit();
        SDL_DestroyWindow(app->window);
        SDL_Quit();
        return;
    }

    // --- VSync --- (0 = off, 1 = on, -1 = adaptive)
    if (SDL_GL_SetSwapInterval(1) < 0) {
         printf("[WARN] Unable to set VSync: %s\n", SDL_GetError());
    }

    // --- Shaders ---
    app->shader_program = load_shaders("shaders/simple.vert", "shaders/simple.frag");
    if (app->shader_program == 0) {
        printf("[ERROR] Failed to load shaders. Exiting.\n");
        // Perform cleanup similar to other init failures
        SDL_GL_DeleteContext(app->gl_context);
        IMG_Quit();
        SDL_DestroyWindow(app->window);
        SDL_Quit();
        return;
    }
    cache_shader_uniform_locations(app);
    
    // --- Dear ImGui ---
    printf("DEBUG: init_app - Initializing ImGui...\n");
    
    // Initialize ImGui using the C wrapper
    if (!ImGui_InitWrapper(app->window, app->gl_context)) {
        printf("[ERROR] Failed to initialize ImGui. Exiting.\n");
        glDeleteProgram(app->shader_program);
        SDL_GL_DeleteContext(app->gl_context);
        IMG_Quit();
        SDL_DestroyWindow(app->window);
        SDL_Quit();
        return;
    }
    printf("DEBUG: init_app - ImGui Initialized.\n");

    // --- Core GL State ---
    init_opengl();
    reshape(width, height); // Set initial viewport

    // --- Initialize Game State ---
    printf("DEBUG: init_app - Initializing Game State...\n");
    app->game_state.current_phase = PHASE_PREPARE;
    app->game_state.player_hp = 100; // Starting HP
    app->game_state.player_gold = 10;  // Starting Gold
    app->game_state.current_wave = 1;
    app->game_state.combat_phase_timer = 0.0f;
    printf("DEBUG: init_app - Initial Game State: Phase=%d, HP=%d, Gold=%d, Wave=%d\n",
           app->game_state.current_phase, app->game_state.player_hp,
           app->game_state.player_gold, app->game_state.current_wave);
    
    // --- Game State Init ---
    vec3 board_center = {
            (BOARD_GRID_WIDTH * BOARD_TILE_SIZE) / 2.0f,
            CAMERA_TARGET_Y_OFFSET, // Look at a specific height
            (BOARD_GRID_HEIGHT * BOARD_TILE_SIZE) / 2.0f
    };
    init_camera(&(app->camera), board_center); // Pass initial target
    init_scene(&(app->scene));
    printf("DEBUG: init_app - Checking VAO ID immediately after init_scene: %u\n", app->scene.board.model.vao_id);
    if(app->scene.board.model.vao_id == 0) {
        printf("[CRITICAL ERROR] VAO ID is 0 immediately after init_scene.\n");
    }

    // --- Matrices ---
    glm_mat4_identity(app->projection_matrix);
    glm_mat4_identity(app->view_matrix);
    
    app->is_running = true;
    
    printf("DEBUG: init_app - END\n");
}

void init_opengl()
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // --- Alpha Blending ---
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    printf("DEBUG: Alpha blending enabled.\n");
}

void reshape(GLsizei width, GLsizei height)
{
    // Prevent division by zero if height is 0
    if (height == 0) height = 1;

    glViewport(0, 0, width, height); // Set the OpenGL viewport
}

void process_game_input_and_logic(App* app) {
    // --- Camera Rotation ---
    static bool is_camera_rotating_with_mouse = false; // State for mouse-driven rotation
    static bool ignore_first_mouse_delta_after_capture = false;
    static int stored_mouse_x_before_rotation;
    static int stored_mouse_y_before_rotation;

    // Use right mouse button for camera rotation
    if (app->input_state.right_mouse_pressed && !app->input_state.imgui_wants_mouse) {
        if (!is_camera_rotating_with_mouse) { // Check if we weren't already rotating
            is_camera_rotating_with_mouse = true;
            SDL_GetMouseState(&stored_mouse_x_before_rotation, &stored_mouse_y_before_rotation);
            SDL_SetRelativeMouseMode(SDL_TRUE);
            ignore_first_mouse_delta_after_capture = true; // Set flag to ignore next delta
            // printf("DEBUG: Camera rotation started (Relative Mouse ON)\n");
        }
    }
    
    if (app->input_state.right_mouse_released) {
        if (is_camera_rotating_with_mouse) {
            is_camera_rotating_with_mouse = false;
            SDL_SetRelativeMouseMode(SDL_FALSE);
            SDL_WarpMouseInWindow(app->window, stored_mouse_x_before_rotation, stored_mouse_y_before_rotation);
            // printf("DEBUG: Camera rotation stopped (Relative Mouse OFF)\n");
        }
    }

    if (is_camera_rotating_with_mouse) {
        int mouse_dx, mouse_dy;
        SDL_GetRelativeMouseState(&mouse_dx, &mouse_dy); // Get motion since last call

        if (ignore_first_mouse_delta_after_capture) {
            // printf("DEBUG: Ignoring first mouse delta: dx=%d, dy=%d\n", mouse_dx, mouse_dy);
            ignore_first_mouse_delta_after_capture = false; // Reset flag for next time
        } else {
            if (mouse_dx != 0 || mouse_dy != 0) {
                rotate_camera(&app->camera, (double)mouse_dx, (double)mouse_dy);
            }
        }
    }

    // --- Camera Zoom ---
    if (app->input_state.mouse_wheel_delta_y != 0.0f && !app->input_state.imgui_wants_mouse) {
        zoom_camera(&app->camera, app->input_state.mouse_wheel_delta_y);
    }

    // --- Camera Panning ---
    // The InputState now holds the *intent* to pan based on W/A/S/D
    // The camera's internal speed is set by set_camera_pan_speed
    // update_camera() will then use this speed to move.
    set_camera_pan_speed(&app->camera,
                         app->input_state.pan_forward_backward_intent,
                         app->input_state.pan_right_left_intent);

    // --- Unit Placement Logic (triggered by left mouse press) ---
    if (app->input_state.left_mouse_pressed && !app->input_state.imgui_wants_mouse) {
        printf("DEBUG: Left Mouse Pressed for Game Logic. Selected bench index: %d\n", app->selected_bench_unit_index);
        if (app->game_state.current_phase == PHASE_PREPARE) {
            if (app->selected_bench_unit_index != -1) { // A unit is "picked up" from bench
                printf("DEBUG: Attempting to place unit from bench index %d\n", app->selected_bench_unit_index);
                if (app->input_state.is_mouse_over_board) {
                    printf("DEBUG: Mouse is over board at (%d, %d)\n", app->input_state.hovered_grid_x, app->input_state.hovered_grid_y);

                    bool on_player_side = is_tile_on_player_side(app->input_state.hovered_grid_y);
                    bool tile_empty = !is_tile_occupied(&app->scene, app->input_state.hovered_grid_x, app->input_state.hovered_grid_y, NULL);

                    printf("DEBUG: Placing at (%d, %d). Player side: %s, Tile empty: %s\n",
                           app->input_state.hovered_grid_x, app->input_state.hovered_grid_y,
                           on_player_side ? "Yes" : "No", tile_empty ? "Yes" : "No");

                    if (on_player_side && tile_empty) {
                        // Valid placement
                        // Bounds check for selected_bench_unit_index against scene.unit_count
                        if (app->selected_bench_unit_index >= 0 && app->selected_bench_unit_index < app->scene.unit_count) {
                            Unit* unit_to_place = &app->scene.units[app->selected_bench_unit_index];
                            printf("DEBUG: Placing Unit Type %d (ID: %d) from units array index %d onto board.\n",
                                   unit_to_place->type, unit_to_place->id, app->selected_bench_unit_index);

                            unit_to_place->location = LOC_BOARD;
                            unit_to_place->grid_x = app->input_state.hovered_grid_x;
                            unit_to_place->grid_y = app->input_state.hovered_grid_y;
                            grid_to_world_pos(unit_to_place->grid_x, unit_to_place->grid_y, unit_to_place->world_pos);
                            unit_to_place->world_pos[1] = 0.1f; // Ensure it's slightly above ground

                            app->selected_bench_unit_index = -1; // Deselect, unit is placed
                            printf("DEBUG: Unit placed successfully.\n");
                        } else {
                            printf("ERROR: Invalid selected_bench_unit_index: %d (unit_count: %d)\n", app->selected_bench_unit_index, app->scene.unit_count);
                            app->selected_bench_unit_index = -1; // Reset to avoid further errors
                        }
                    } else {
                        printf("DEBUG: Invalid placement location. Unit remains selected.\n");
                        // Optionally, deselect the unit if placement fails:
                        // app->selected_bench_unit_index = -1;
                    }
                } else {
                    printf("DEBUG: Clicked outside board while placing. Unit remains selected.\n");
                    // Optionally deselect if clicked outside board:
                    // app->selected_bench_unit_index = -1;
                }
            } else {
                printf("DEBUG: Left click on world, but no unit selected from bench for placement.\n");
            }    
        } else {
            printf("DEBUG: Placement attempted outside Prepare phase.\n");
        }
    }

    if (app->input_state.right_mouse_pressed && !app->input_state.imgui_wants_mouse) {
        if (app->selected_bench_unit_index != -1) { // If a unit is currently "picked up" for placement
            printf("DEBUG: Placement cancelled with right click. Unit %d returned to bench selection.\n", app->selected_bench_unit_index);
            app->selected_bench_unit_index = -1; // Deselect the unit
            // is_camera_rotating_with_mouse will be set by the camera rotation block, so right-click still initiates rotation
        }
        // If no unit is selected, right-click will just initiate camera rotation as per existing logic.
    }

    if (app->input_state.f1_pressed_this_frame && !app->input_state.imgui_wants_keyboard) {
        app->show_help_window = !app->show_help_window; // Toggle help window
        printf("DEBUG: F1 pressed. Show help: %s\n", app->show_help_window ? "Yes" : "No");
    }

    // --- Light Adjustment Example ---
    // Assuming these flags are set in InputState by InputManager_PollAndProcess for SDL_SCANCODE_KP_PLUS / MINUS
    if (app->input_state.plus_key_pressed && !app->input_state.imgui_wants_keyboard) {
     app->light_direction_world[2] += 0.1f; // Move light along Z
     if (app->light_direction_world[2] > 1.0f) app->light_direction_world[2] = 1.0f;
     glm_vec3_normalize(app->light_direction_world);
     printf("DEBUG: Light Dir Z: %.2f\n", app->light_direction_world[2]);
    }
    if (app->input_state.minus_key_pressed && !app->input_state.imgui_wants_keyboard) {
     app->light_direction_world[2] -= 0.1f;
     if (app->light_direction_world[2] < -1.0f) app->light_direction_world[2] = -1.0f;
     glm_vec3_normalize(app->light_direction_world);
    printf("DEBUG: Light Dir Z: %.2f\n", app->light_direction_world[2]);
    }
    
    // --- Other general key presses for single actions ---
}

void update_app(App* app) {
    static Uint64 last_counter = 0;
    Uint64 current_counter = SDL_GetPerformanceCounter();
    double elapsed_time = 0.0;

    if (last_counter != 0) {
        elapsed_time = (double)(current_counter - last_counter) / SDL_GetPerformanceFrequency();
    }
    last_counter = current_counter;

    if (elapsed_time > 0.1) elapsed_time = 0.1; // Clamp dt
    app->uptime += elapsed_time;

    // --- Process Input and Game Logic ---
    InputManager_PollAndProcess(app, &app->input_state);

    if (app->input_state.quit_requested) {
        app->is_running = false;
        return;
    }

    process_game_input_and_logic(app); // Handle actions based on polled input

    // --- Game Phase Logic ---
    if (app->game_state.player_hp <= 0 && app->game_state.current_phase != PHASE_GAME_OVER) {
        printf("DEBUG: Player HP <= 0. GAME OVER.\n");
        app->game_state.current_phase = PHASE_GAME_OVER;
    }

    // Handle different phases (except if game over)
    if (app->game_state.current_phase != PHASE_GAME_OVER) {
        switch (app->game_state.current_phase) {
            case PHASE_PREPARE:
                // Logic for prepare phase (e.g., timers, auto-start?) currently handled by UI interaction
                break;
            case PHASE_COMBAT:
                app->game_state.combat_phase_timer += (float)elapsed_time;
                const float COMBAT_DURATION = 15.0f; // Longer combat for testing damage
                bool all_player_units_dead = true;
                bool all_ai_units_dead = true;

                // Check win/loss conditions even before timer runs out
                for(int i=0; i < app->scene.unit_count; ++i) {
                    if(app->scene.units[i].location == LOC_BOARD && app->scene.units[i].is_alive) {
                        if(app->scene.units[i].is_player_unit) all_player_units_dead = false;
                        else all_ai_units_dead = false;
                    }
                }

                if (all_player_units_dead || all_ai_units_dead || app->game_state.combat_phase_timer >= COMBAT_DURATION) {
                    printf("DEBUG: Combat Phase Ended. Timer: %.2f, PlayerDead: %d, AIDead: %d\n",
                           app->game_state.combat_phase_timer, all_player_units_dead, all_ai_units_dead);
                    app->game_state.current_phase = PHASE_POST_COMBAT;
                    app->game_state.combat_phase_timer = 0.0f;
                }
                break;
            case PHASE_POST_COMBAT:
                printf("DEBUG: Post-Combat Phase. Processing results.\n");
                bool player_won_round = false;
                bool ai_won_round = false;
                int surviving_ai_count_for_damage = 0;

                // Determine combat outcome
                bool player_units_remain = false;
                bool ai_units_remain = false;
                for(int i=0; i < app->scene.unit_count; ++i) {
                    if(app->scene.units[i].location == LOC_BOARD && app->scene.units[i].is_alive) {
                        if(app->scene.units[i].is_player_unit) player_units_remain = true;
                        else {
                            ai_units_remain = true;
                            surviving_ai_count_for_damage++; // Count for player damage
                        }
                    }
                }

                if (player_units_remain && !ai_units_remain) {
                    player_won_round = true;
                    printf("DEBUG: Player WON the round!\n");
                    app->game_state.player_gold += 3; // Bonus gold for winning
                } else if (!player_units_remain && ai_units_remain) {
                    ai_won_round = true;
                    printf("DEBUG: Player LOST the round (all player units died)!\n");
                } else if (!player_units_remain && !ai_units_remain) {
                    printf("DEBUG: DRAW - All units died.\n");
                } else { // Both sides have units, or combat timer ended with survivors on both
                    ai_won_round = true; // Assume player loses if combat times out with AI survivors
                    printf("DEBUG: Combat ended (timer or mutual survivors), AI considered winner for damage.\n");
                }

                // Apply damage to player if AI won or combat timed out with AI survivors
                if (ai_won_round && surviving_ai_count_for_damage > 0) {
                    int damage_to_player = surviving_ai_count_for_damage * 5; // Example: 5 HP per surviving AI
                    damage_to_player += app->game_state.current_wave; // Bonus damage for wave number
                    app->game_state.player_hp -= damage_to_player;
                    printf("DEBUG: Player takes %d damage. Player HP: %d\n", damage_to_player, app->game_state.player_hp);
                }


                app->game_state.player_gold += 5; // Base gold income
                app->game_state.current_wave++;
                printf("DEBUG: Next Round - Wave: %d, Gold: %d, HP: %d\n",
                       app->game_state.current_wave, app->game_state.player_gold, app->game_state.player_hp);

                // Clean up units for next round
                int new_unit_count = 0;
                for (int i = 0; i < app->scene.unit_count; ++i) {
                    Unit* unit = &app->scene.units[i];
                    if (unit->is_player_unit) { // Keep player units
                        if (unit->location == LOC_BOARD && !unit->is_alive) {
                            // Player unit died on board, move it to bench (or remove?)
                            // For now, let's assume it's "removed" from active play this round
                            // but could be re-bought or revived (not implemented).
                            // Or, for simplicity, just remove if dead.
                            // Let's make dead player units simply inactive for now.
                            // They won't be copied to new_unit_count if dead.
                            // OR: if player unit dies, it is GONE.
                            // Let's go with: player units that die are removed from the board list.
                            // Player units on bench are untouched.
                            printf("DEBUG: Player unit %d died and is removed from board consideration.\n", unit->id);
                            unit->location = LOC_NONE; // Mark as inactive
                        } else if (unit->location == LOC_BENCH || (unit->location == LOC_BOARD && unit->is_alive)) {
                            // Reset active player units for next round
                            unit->current_hp = unit->max_hp;
                            unit->is_alive = true;
                            unit->current_combat_state = UNIT_STATE_IDLE;
                            unit->current_target_ptr = NULL;
                            unit->attack_cooldown_timer = 0.0f;
                            if (new_unit_count != i) { // Compact the array
                                app->scene.units[new_unit_count] = *unit;
                            }
                            new_unit_count++;
                        }
                    }
                    // AI units are implicitly removed by not being copied
                }
                app->scene.unit_count = new_unit_count;
                printf("DEBUG: Active player units for next round: %d\n", app->scene.unit_count);


                app->game_state.current_phase = PHASE_PREPARE;
                printf("DEBUG: Transitioning to Prepare Phase.\n");
                break;
            case PHASE_GAME_OVER:
                // Handled by the initial check, just break here.
                break;
        }
    }

    // --- Update Game Systems ---
    update_camera(&(app->camera), elapsed_time);
    update_scene(&(app->scene), (float)elapsed_time, app->game_state.current_phase);
    

    // --- Update Matrices ---
    calculate_view_matrix(&app->camera, app->view_matrix);
    int width, height;
    SDL_GL_GetDrawableSize(app->window, &width, &height);
    if (height == 0) height = 1;
    float aspect = (float)width / (float)height;
    glm_perspective(glm_rad(VIEWPORT_ASPECT), aspect, 0.1f, 100.0f, app->projection_matrix);
}

void render_app(App* app)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    check_gl_error("glClear - render_app start");

    ImGui_NewFrameWrapper();

    if (app->shader_program != 0) {
        // printf("DEBUG: render_app - Using program %u for 3D scene.\n", app->shader_program);
        glUseProgram(app->shader_program);
        check_gl_error("render_app - AFTER glUseProgram for 3D scene");

        GLint active_program_check1 = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &active_program_check1);
        // printf("DEBUG: render_app - Active program after explicit use: %d (Expected: %u)\n", active_program_check1, app->shader_program);
        if (active_program_check1 != app->shader_program) {
            printf("CRITICAL ERROR: Shader program %u is NOT active after glUseProgram!\n", app->shader_program);
        }

        // Set View, Projection matrices
        if (app->shader_uloc_projection != -1) glUniformMatrix4fv(app->shader_uloc_projection, 1, GL_FALSE, (const GLfloat*)app->projection_matrix);
        check_gl_error("render_app - Proj Uniform");
        if (app->shader_uloc_view != -1) glUniformMatrix4fv(app->shader_uloc_view, 1, GL_FALSE, (const GLfloat*)app->view_matrix);
        check_gl_error("render_app - View Uniform");

        // Set Texture Sampler
        if (app->shader_uloc_texture1 != -1) glUniform1i(app->shader_uloc_texture1, 0);
        check_gl_error("render_app - Tex1 Uniform");

        // printf("DEBUG: render_app - Setting lightDir_world. Location: %d\n", app->shader_uloc_lightDir);
        if (app->shader_uloc_lightDir != -1) {
            glUniform3fv(app->shader_uloc_lightDir, 1, app->light_direction_world);
            check_gl_error("render_app - glUniform3fv for lightDir_world"); // Check this one specifically
        }

        // printf("DEBUG: render_app - Setting lightColor. Location: %d\n", app->shader_uloc_lightColor);
        if (app->shader_uloc_lightColor != -1) {
            glUniform3fv(app->shader_uloc_lightColor, 1, app->light_color);
            check_gl_error("render_app - glUniform3fv for lightColor"); // Error starts here or after
        }

        // printf("DEBUG: render_app - Setting ambientLightColor. Location: %d\n", app->shader_uloc_ambientLightColor);
        if (app->shader_uloc_ambientLightColor != -1) {
            glUniform3fv(app->shader_uloc_ambientLightColor, 1, app->ambient_light_color);
            check_gl_error("render_app - glUniform3fv for ambientLightColor");
        }

        // printf("DEBUG: render_app - Setting viewPos_world. Location: %d\n", app->shader_uloc_viewPos);
        if (app->shader_uloc_viewPos != -1) {
            glUniform3fv(app->shader_uloc_viewPos, 1, app->camera.position);
            check_gl_error("render_app - glUniform3fv for viewPos_world");
        }

        // Default color tint
        if (app->shader_uloc_color_tint != -1) {
            glUniform4f(app->shader_uloc_color_tint, 1.0f, 1.0f, 1.0f, 1.0f);
        }
        check_gl_error("render_app - Set default color tint");

        render_scene(&(app->scene), app->shader_program, app, app->selected_bench_unit_index);
        check_gl_error("render_app - after render_scene");

    } else {
        printf("[ERROR] render_app: Invalid shader program ID! Cannot render 3D scene.\n");
    }
    
    render_scene(&(app->scene), app->shader_program, app, app->selected_bench_unit_index);
    check_gl_error("after render_scene");

    // --- Draw ImGui Game UI ---
    ImGui_DrawPlayerStatsWindowWrapper(
            &app->game_state,
            app->input_state.is_mouse_over_board,
            app->input_state.hovered_grid_x,
            app->input_state.hovered_grid_y
    );
    
    
    int shop_actions = ACTION_FLAG_NONE;

    if (app->game_state.current_phase == PHASE_PREPARE) {
        shop_actions = ImGui_DrawShopWindowWrapper(&app->game_state);
        ImGui_DrawBenchWindowWrapper(&app->scene, &app->selected_bench_unit_index, app->game_state.current_phase);
    } else if (app->game_state.current_phase == PHASE_COMBAT) {
        ImGui_DrawCombatInfoWindowWrapper(&app->game_state);
    } else if (app->game_state.current_phase == PHASE_POST_COMBAT) {
        ImGui_DrawPostCombatWindowWrapper(&app->game_state);
    } else if (app->game_state.current_phase == PHASE_GAME_OVER) {
        if (ImGui_DrawGameOverWindowWrapper(&app->game_state)) {
            // Handle restart logic if button returns true
            // For now, just prints a message. Later, re-initialize app state.
            printf("INFO: Restart action triggered from Game Over screen.\n");
            // init_app(app, initial_width, initial_height); // Example of a full restart
            // Or just reset game_state and scene for a new game.
        }
    }

    if (app->show_help_window) {
        ImGui_DrawHelpWindowWrapper(&app->show_help_window);
    }
    
    // --- Handle UI Actions (Primarily Shop and Start Combat) ---
    // These actions should only be processed if they could have been triggered (e.g., in Prepare Phase)
    if (app->game_state.current_phase == PHASE_PREPARE && shop_actions != ACTION_FLAG_NONE) {
        printf("DEBUG: Shop action flags received: %d\n", shop_actions);

        // Handle Start Combat action
        if (shop_actions & ACTION_FLAG_START_COMBAT) {
            printf("Action: Start Combat triggered!\n");

            // --- Spawn AI Wave ---
            printf("DEBUG: Spawning AI wave %d...\n", app->game_state.current_wave);
            // For now, spawn simple fixed AI wave.
            // Ensure player units are not overwritten if MAX_UNITS is tight.
            // This assumes add_unit_to_bench finds slots in the combined units array.
            // Better: add_unit_to_board_directly function or use a different list for active combatants.

            // Example: Spawn 2 AI units (make sure MAX_UNITS allows this)
            // These will be added to scene.units and their location set to LOC_BOARD
            // We need a robust way to add them to the board, let's modify init_unit to handle this.
            // And ensure existing AI from previous rounds are cleared.
            // Clearing AI is now done in PHASE_POST_COMBAT logic.

            if (app->scene.unit_count < MAX_UNITS) {
                init_unit(&app->scene.units[app->scene.unit_count++], UNIT_MELEE_TANK, 3, BOARD_GRID_HEIGHT - 2, false, LOC_BOARD);
            }
            if (app->scene.unit_count < MAX_UNITS) {
                init_unit(&app->scene.units[app->scene.unit_count++], UNIT_RANGED_ARCHER, 4, BOARD_GRID_HEIGHT - 2, false, LOC_BOARD);
            }
            printf("DEBUG: AI units spawned. Total units: %d\n", app->scene.unit_count);

            app->game_state.current_phase = PHASE_COMBAT;
            app->game_state.combat_phase_timer = 0.0f;
            app->selected_bench_unit_index = -1;
        }

        // --- Handle Buy actions using new flags ---
        if (shop_actions & ACTION_FLAG_BUY_UNIT_TYPE_0) { // Buy Tank
            int cost = get_unit_cost(UNIT_MELEE_TANK); // Get cost
            printf("Action: Attempting to buy Tank (Cost: %d, Current Gold: %d)...\n", cost, app->game_state.player_gold);
            if (app->game_state.player_gold >= cost) { // Check gold
                if (add_unit_to_bench(&app->scene, UNIT_MELEE_TANK)) {
                    app->game_state.player_gold -= cost; // Deduct gold
                    printf("Purchase successful! Gold remaining: %d\n", app->game_state.player_gold);
                } else {
                    printf("Action Failed: Bench is full or max units reached for Tank!\n");
                    // No gold deducted if add_unit_to_bench fails
                }
            } else {
                printf("Action Failed: Not enough gold for Tank!\n");
            }
        }
        if (shop_actions & ACTION_FLAG_BUY_UNIT_TYPE_1) { // Buy Archer
            int cost = get_unit_cost(UNIT_RANGED_ARCHER); // Get cost
            printf("Action: Attempting to buy Archer (Cost: %d, Current Gold: %d)...\n", cost, app->game_state.player_gold);
            if (app->game_state.player_gold >= cost) { // Check gold
                if (add_unit_to_bench(&app->scene, UNIT_RANGED_ARCHER)) {
                    app->game_state.player_gold -= cost; // Deduct gold
                    printf("Purchase successful! Gold remaining: %d\n", app->game_state.player_gold);
                } else {
                    printf("Action Failed: Bench is full or max units reached for Archer!\n");
                }
            } else {
                printf("Action Failed: Not enough gold for Archer!\n");
            }
        }
        // Add more unit type buy handlers if shop expands

        // Handle Refresh action
        if (shop_actions & ACTION_FLAG_REFRESH_SHOP) {
            int cost = 1; // Example refresh cost
            printf("Action: Attempting Refresh Shop (Cost: %d, Current Gold: %d)...\n", cost, app->game_state.player_gold);
            if (app->game_state.player_gold >= cost) { // Check gold
                app->game_state.player_gold -= cost; // Deduct gold
                printf("Refresh successful! Gold remaining: %d\n", app->game_state.player_gold);
                // TODO: Implement logic to change shop offerings
            } else {
                printf("Action Failed: Not enough gold to refresh shop!\n");
            }
        }
    }


    // --- Render ImGui Draw Data ---
    ImGui_RenderWrapper();
    check_gl_error("ImGui_RenderWrapper");
    ImGui_RenderDrawDataWrapper();
    check_gl_error("ImGui_RenderDrawDataWrapper");

    SDL_GL_SwapWindow(app->window);
}

void destroy_app(App* app)
{
    printf("DEBUG: destroy_app - START\n");

    // --- Shutdown ImGui ---
    printf("DEBUG: destroy_app - Shutting down ImGui...\n");
    ImGui_ShutdownWrapper();
    printf("DEBUG: destroy_app - ImGui shutdown complete.\n");

    // Delete shader program
    if (app->shader_program != 0) {
        glDeleteProgram(app->shader_program);
    }

    // Destroy scene resources
    destroy_scene(&app->scene);

    // SDL cleanup
    printf("DEBUG: destroy_app - Calling SDL cleanup...\n");
    if (app->gl_context != NULL) { SDL_GL_DeleteContext(app->gl_context); }
    if (app->window != NULL) { SDL_DestroyWindow(app->window); }
    IMG_Quit(); // Still here if used by texture loader
    SDL_Quit();
    printf("DEBUG: destroy_app - END\n");
}