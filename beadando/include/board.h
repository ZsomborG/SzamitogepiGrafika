#ifndef BOARD_H
#define BOARD_H

#include <obj/model.h>
#include <cglm/cglm.h>
#include <glad/glad.h>
#include "unit.h"
#include <stdbool.h>

struct Scene;
struct App;

#define BOARD_GRID_WIDTH 8
#define BOARD_GRID_HEIGHT 8

#define BOARD_TILE_SIZE 1.0f

typedef struct {
    Model model;
    GLuint texture_id;
} Board;

bool init_board(Board* board, const char* model_path, const char* texture_path);

void render_board(const Board* board, GLuint shader_program, const struct App* app);

void destroy_board(Board* board);

void grid_to_world_pos(int grid_x, int grid_y, vec3 world_pos);

bool world_to_grid_pos(vec3 world_pos, int* grid_x, int* grid_y);

/**
 * @brief Checks if the given grid Y-coordinate is on the player's side of the board.
 * (Example: bottom half of the board)
 */
bool is_tile_on_player_side(int grid_y);

/**
 * @brief Checks if the specified grid tile is currently occupied by any live unit on the board.
 * @param scene Pointer to the main Scene containing all unit data.
 * @param grid_x The x-coordinate of the tile to check.
 * @param grid_y The y-coordinate of the tile to check.
 * @param occupying_unit_ptr Optional: If not NULL and tile is occupied,
 *                           this will be set to point to the occupying unit.
 * @return true if the tile is occupied, false otherwise.
 */
bool is_tile_occupied(const struct Scene* scene, int grid_x, int grid_y, const Unit** occupying_unit_ptr);

bool is_tile_empty_for_player(const struct Scene* scene, int grid_x, int grid_y);

#endif // BOARD_H