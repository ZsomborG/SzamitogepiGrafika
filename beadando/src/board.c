#include "board.h"
#include "texture.h"
#include "scene.h"
#include "unit.h"
#include "app.h"

#include <obj/load.h>
#include <obj/model.h>
#include <obj/draw.h>

#include <stdio.h>
#include <stdbool.h>

bool init_board(Board* board, const char* model_path, const char* texture_path){
    printf("DEBUG: init_board - START (Model: %s, Texture: %s)\n", model_path, texture_path);
    if(!board || !model_path || !texture_path) {
        fprintf(stderr, "ERROR: init_board - Invalid arguments.\n");
        return FALSE;
    }

    init_model(&board->model);
    
    // 1. Load model
    printf("DEBUG: init_board - Loading board model...\n");
    if (!load_model(&board->model, model_path)){
        fprintf(stderr, "ERROR: init_board - Failed to load board model '%s'.\n", model_path);
        return FALSE;
    }
    
    // 2. Setup Model Buffers (VAO/VBO/IBO)
    printf("DEBUG: init_board - Setting up model buffers...\n");
    // Check if model loading actually produced vertices/triangles
    if(board->model.n_vertices == 0 || board->model.n_triangles == 0){
        fprintf(stderr, "ERROR: init_board - Board model loaded but has no vertices or triangles.\n");
        free_model(&board->model);
        return FALSE;
    }
    setup_model_buffers(&board->model);
    // Check if buffer setup actually created a VAO
    if(board->model.vao_id == 0){
        fprintf(stderr, "ERROR: init_board - Board model buffer setup failed.\n");
        free_model(&board->model);
        return FALSE;
    }
    
    // 3. Load texture
    printf("DEBUG: init_board - Loading board texture...\n");
    board->texture_id = load_texture(texture_path);
    if (board->texture_id == 0) {
        fprintf(stderr, "ERROR: init_board - Failed to load board texture '%s'.\n", texture_path);
        destroy_model_buffers(&board->model); // Clean up VAO/VBO/IBO
        free_model(&board->model);     // Clean up model
        return FALSE;
    }
    printf("DEBUG: init_board - Board initialized successfully.\n");
    return TRUE;
}

void render_board(const Board* board, GLuint shader_program, const struct App* app) { // Signature updated
    if (!board || board->model.vao_id == 0 || shader_program == 0 || !app) {
        // printf("DEBUG: render_board - SKIPPING (Invalid board/vao/shader/app)\n");
        return;
    }
    // printf("DEBUG: render_board - START (VAO: %u, TexID: %u, Shader: %u)\n", board->model.vao_id, board->texture_id, shader_program);

    mat4 model_matrix;
    glm_mat4_identity(model_matrix);
    float scale_x = BOARD_GRID_WIDTH * BOARD_TILE_SIZE;
    float scale_z = BOARD_GRID_HEIGHT * BOARD_TILE_SIZE;
    glm_translate(model_matrix, (vec3){scale_x / 2.0f, 0.0f, scale_z / 2.0f});
    glm_scale(model_matrix, (vec3){scale_x, 1.0f, scale_z});

    // --- Fetch model_uloc from app ---
    GLint model_uloc = app->shader_uloc_model;
    if (model_uloc != -1) {
        glUniformMatrix4fv(model_uloc, 1, GL_FALSE, (const GLfloat*)model_matrix);
    } else { /* printf("[WARN] render_board: Invalid model uniform location in app struct!\n"); */ }

    // --- Set Material Uniforms for the Board ---
    if (app->shader_uloc_materialDiffuse != -1) glUniform3fv(app->shader_uloc_materialDiffuse, 1, app->scene.material.diffuse);
    if (app->shader_uloc_materialSpecular != -1) glUniform3fv(app->shader_uloc_materialSpecular, 1, app->scene.material.specular);
    if (app->shader_uloc_materialShininess != -1) glUniform1f(app->shader_uloc_materialShininess, app->scene.material.shininess);
    check_gl_error("Set board material uniforms");

    glBindTexture(GL_TEXTURE_2D, board->texture_id);
    draw_model(&(board->model));
    // printf("DEBUG: render_board - END\n");
}


void destroy_board(Board* board){
    // printf("DEBUG: destroy_board - START\n");
    
    if(board) {
        destroy_model_buffers(&board->model); // Clean up VAO/VBO/IBO
        // free_model(&board->model);     // Clean up model
        if(board->texture_id != 0){
            glDeleteTextures(1, &board->texture_id); // Clean up texture
            board->texture_id = 0;
        }
    }
    // printf("DEBUG: destroy_board - END\n");
}

void grid_to_world_pos(int grid_x, int grid_y, vec3 world_pos) {
    // calculate the center of the tile
    // assuming grid (0,0) corresponds to world origin (0,0,0) for now
    // adjust if board model origin or world origin differs
    world_pos[0] = ((float)grid_x + 0.5f) * BOARD_TILE_SIZE;
    world_pos[1] = 0.0f;
    world_pos[2] = ((float)grid_y + 0.5f) * BOARD_TILE_SIZE;
}

bool world_to_grid_pos(vec3 world_pos, int* grid_x, int* grid_y) {
    if (!grid_x || !grid_y) {
        return false;
    }
    
    *grid_x = (int)floor(world_pos[0] / BOARD_TILE_SIZE);
    *grid_y = (int)floor(world_pos[2] / BOARD_TILE_SIZE);
    
    if (*grid_x < 0 || *grid_x >= BOARD_GRID_WIDTH || *grid_y < 0 || *grid_y >= BOARD_GRID_HEIGHT) {
        return false;
    }
    
    return true;
}

bool is_tile_on_player_side(int grid_y) {
    // Example: Player's side is the bottom half of the board
    return (grid_y < BOARD_GRID_HEIGHT / 2);
}

bool is_tile_occupied(const struct Scene* scene, int grid_x, int grid_y, const Unit** occupying_unit_ptr) {
    if (occupying_unit_ptr) *occupying_unit_ptr = NULL; // Initialize output param
    if (!scene) return false; // Or true, depending on desired behavior for null scene

    for (int i = 0; i < scene->unit_count; ++i) {
        const Unit* unit = &scene->units[i];
        if (unit->is_alive && unit->location == LOC_BOARD &&
            unit->grid_x == grid_x && unit->grid_y == grid_y) {
            if (occupying_unit_ptr) *occupying_unit_ptr = unit;
            return true; // Tile is occupied
        }
    }
    return false; // Tile is empty
}

// Reimplement is_tile_empty_for_player using the new function
bool is_tile_empty_for_player(const struct Scene* scene, int grid_x, int grid_y) {
    const Unit* occupying_unit = NULL;
    if (is_tile_occupied(scene, grid_x, grid_y, &occupying_unit)) {
        if (occupying_unit && occupying_unit->is_player_unit) {
            return false; // Occupied by a player unit
        }
    }
    return true; // Empty or occupied by non-player unit (which player can place on top of initially)
    // Or for placement, you might want it to be strictly empty of ANY unit:
    // return !is_tile_occupied(scene, grid_x, grid_y, NULL);
}