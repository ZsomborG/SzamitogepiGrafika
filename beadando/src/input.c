#include "input.h"
#include "app.h"             // For App struct (matrices, camera, window)
#include "board.h"           // For BOARD_GRID_WIDTH/HEIGHT, world_to_grid_pos
#include "camera.h"          // For CAMERA_PAN_THRESHOLD
#include "imgui_interface.h" // For ImGui event processing and focus checks

#include <stdio.h> // For debug

// Helper: Ray-Plane Intersection (copied from app.c, could be in a math_utils.c)
static bool ray_plane_intersection_input(vec3 ray_origin, vec3 ray_direction, vec3 plane_point, vec3 plane_normal, vec3 out_intersection_point) {
    float denominator = glm_vec3_dot(plane_normal, ray_direction);
    if (fabsf(denominator) < 1e-6) return false;
    vec3 origin_to_plane;
    glm_vec3_sub(plane_point, ray_origin, origin_to_plane);
    float t = glm_vec3_dot(origin_to_plane, plane_normal) / denominator;
    // if (t < 0) return false; // Optional: Check if intersection is in front
    vec3 t_direction;
    glm_vec3_scale(ray_direction, t, t_direction);
    glm_vec3_add(ray_origin, t_direction, out_intersection_point);
    return true;
}

// Helper: Calculate Mouse Ray (copied from app.c)
static void calculate_mouse_ray_input(App* app, int screen_x, int screen_y, vec3 out_ray_origin, vec3 out_ray_direction) {
    int win_width, win_height;
    SDL_GetWindowSize(app->window, &win_width, &win_height);
    if (win_width == 0 || win_height == 0) return;

    float ndc_x = (2.0f * (float)screen_x) / (float)win_width - 1.0f;
    float ndc_y = 1.0f - (2.0f * (float)screen_y) / (float)win_height;

    vec4 clip_coords = {ndc_x, ndc_y, -1.0f, 1.0f};
    mat4 inv_projection;
    glm_mat4_inv(app->projection_matrix, inv_projection);
    vec4 eye_coords;
    glm_mat4_mulv(inv_projection, clip_coords, eye_coords);
    eye_coords[2] = -1.0f; eye_coords[3] = 0.0f;

    mat4 inv_view;
    glm_mat4_inv(app->view_matrix, inv_view);
    vec4 world_dir_4;
    glm_mat4_mulv(inv_view, eye_coords, world_dir_4);

    glm_vec3_copy(app->camera.position, out_ray_origin);
    vec3 world_dir_3 = {world_dir_4[0], world_dir_4[1], world_dir_4[2]};
    glm_vec3_normalize_to(world_dir_3, out_ray_direction);
}


void InputManager_PollAndProcess(struct App* app, InputState* input_state) {
    if (!app || !input_state) return;

    // --- Reset per-frame input states ---
    input_state->left_mouse_pressed = false;
    input_state->left_mouse_released = false;
    input_state->right_mouse_pressed = false;
    input_state->right_mouse_released = false;
    input_state->mouse_wheel_delta_y = 0.0f;
    input_state->quit_requested = false;
    input_state->f1_pressed_this_frame = false;
    // hovered_grid_x/y and is_mouse_over_board will be updated later

    // --- Get current continuous states ---
    SDL_GetMouseState(&input_state->mouse_screen_x, &input_state->mouse_screen_y);
    // GetKeyboardState for panning is handled after event polling to combine with presses/releases

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ProcessEventWrapper(&event); // Pass to ImGui first

        switch (event.type) {
            case SDL_QUIT:
                input_state->quit_requested = true;
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    reshape(event.window.data1, event.window.data2);
                }
                break;
            case SDL_MOUSEWHEEL:
                input_state->mouse_wheel_delta_y = (float)event.wheel.y;
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    input_state->left_mouse_pressed = true;
                    input_state->left_mouse_down = true;
                } else if (event.button.button == SDL_BUTTON_RIGHT) {
                    input_state->right_mouse_pressed = true;
                    input_state->right_mouse_down = true;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    input_state->left_mouse_released = true;
                    input_state->left_mouse_down = false;
                } else if (event.button.button == SDL_BUTTON_RIGHT) {
                    input_state->right_mouse_released = true;
                    input_state->right_mouse_down = false;
                }
                break;
            case SDL_KEYDOWN:
                if (event.key.keysym.scancode == SDL_SCANCODE_F1 && !event.key.repeat) {
                    input_state->f1_pressed_this_frame = true;
                }
                break;
        }
    }

    // --- Update ImGui focus state ---
    input_state->imgui_wants_mouse = ImGui_WantCaptureMouseWrapper();
    input_state->imgui_wants_keyboard = ImGui_WantCaptureKeyboardWrapper();

    // --- Handle continuous keyboard state for panning ---
    input_state->pan_forward_backward_intent = 0.0f;
    input_state->pan_right_left_intent = 0.0f;
    const Uint8 *keystate = SDL_GetKeyboardState(NULL);
    if (app->camera.zoom_level < CAMERA_PAN_THRESHOLD && !input_state->imgui_wants_keyboard) {
        if (keystate[SDL_SCANCODE_W]) input_state->pan_forward_backward_intent = -1.0f;
        if (keystate[SDL_SCANCODE_S]) input_state->pan_forward_backward_intent = 1.0f;
        if (keystate[SDL_SCANCODE_A]) input_state->pan_right_left_intent = 1.0f;
        if (keystate[SDL_SCANCODE_D]) input_state->pan_right_left_intent = -1.0f;
    }

    // --- Mouse Picking / Hover Update ---
    vec3 ray_origin, ray_direction;
    calculate_mouse_ray_input(app, input_state->mouse_screen_x, input_state->mouse_screen_y, ray_origin, ray_direction);

    vec3 plane_point = {0.0f, 0.0f, 0.0f};
    vec3 plane_normal = {0.0f, 1.0f, 0.0f};
    vec3 intersection_point;

    if (ray_plane_intersection_input(ray_origin, ray_direction, plane_point, plane_normal, intersection_point)) {
        glm_vec3_copy(intersection_point, input_state->mouse_world_pos_on_plane);
        int grid_x, grid_y;
        if (world_to_grid_pos(intersection_point, &grid_x, &grid_y)) {
            input_state->hovered_grid_x = grid_x;
            input_state->hovered_grid_y = grid_y;
            input_state->is_mouse_over_board = true;
        } else {
            input_state->hovered_grid_x = -1;
            input_state->hovered_grid_y = -1;
            input_state->is_mouse_over_board = false;
        }
    } else {
        input_state->hovered_grid_x = -1;
        input_state->hovered_grid_y = -1;
        input_state->is_mouse_over_board = false;
        glm_vec3_zero(input_state->mouse_world_pos_on_plane);
    }
}