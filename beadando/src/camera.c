#include "camera.h"

#include <glad/glad.h>
#include <cglm/cglm.h>
#include <math.h> // For sin/cos if not using cglm alternatives
#include <stdio.h> // For debug

// --- Constants ---
#define CAMERA_PITCH_MAX 0.0f // Don't allow looking straight up
#define CAMERA_PITCH_MIN (-89.0f) // Limit looking down
#define CAMERA_ROTATE_SENSITIVITY 0.15f
#define CAMERA_ZOOM_SENSITIVITY 0.5f
#define CAMERA_PAN_SPEED 3.0f

// World up direction (Y-up)
vec3 WORLD_UP = {0.0f, 1.0f, 0.0f};

// --- Private Helper Function ---
// Calculates camera position based on target, zoom, yaw, pitch
void calculate_camera_position(Camera* camera) {
    // Calculate offset from target based on spherical coordinates (zoom, yaw, pitch)
    float yaw_rad = glm_rad(camera->rotation[1]);
    float pitch_rad = glm_rad(camera->rotation[0]);

    vec3 offset;
    offset[0] = camera->zoom_level * cos(yaw_rad) * cos(pitch_rad); // Offset X
    offset[1] = -camera->zoom_level * sin(pitch_rad);                 // Offset Y
    offset[2] = camera->zoom_level * sin(yaw_rad) * cos(pitch_rad); // Offset Z

    // Position = Target + Offset
    glm_vec3_add(camera->target_pos, offset, camera->position);
}


// --- Public Functions ---

void init_camera(Camera* camera, vec3 initial_target) {
    if (!camera) return;

    glm_vec3_copy(initial_target, camera->target_pos);

    camera->zoom_level = CAMERA_DEFAULT_ZOOM;
    camera->min_zoom = CAMERA_MIN_ZOOM;
    camera->max_zoom = CAMERA_MAX_ZOOM;

    camera->rotation[0] = CAMERA_DEFAULT_PITCH; // Pitch
    camera->rotation[1] = CAMERA_DEFAULT_YAW;   // Yaw
    camera->rotation[2] = 0.0f;                 // Roll

    glm_vec3_zero(camera->speed); // Panning speed

    camera->is_preview_visible = false;

    // Calculate initial position based on defaults
    calculate_camera_position(camera);
}

// Updates camera based on panning speed
void update_camera(Camera* camera, double time) {
    if (!camera) return;
    float dt = (float)time;

    // Only pan if speed is non-zero AND zoomed in enough
    if ((fabsf(camera->speed[0]) > 0.01f || fabsf(camera->speed[1]) > 0.01f) &&
        camera->zoom_level < CAMERA_PAN_THRESHOLD)
    {
        // Calculate direction vectors based on current rotation
        float yaw_rad = glm_rad(camera->rotation[1]);
        float pitch_rad = glm_rad(camera->rotation[0]);
        vec3 front_dir; // Direction camera is facing
        front_dir[0] = cos(yaw_rad) * cos(pitch_rad);
        front_dir[1] = sin(pitch_rad);
        front_dir[2] = sin(yaw_rad) * cos(pitch_rad);
        glm_vec3_normalize(front_dir);

        vec3 right_dir;
        glm_vec3_crossn(front_dir, WORLD_UP, right_dir);

        // Important: For panning, we often want to move parallel to the ground plane (XZ)
        // regardless of pitch, otherwise looking down makes forward move into the ground.
        // Let's calculate a forward vector projected onto the XZ plane.
        vec3 forward_pan_dir = { cos(yaw_rad), 0.0f, sin(yaw_rad) }; // Use only yaw
        glm_vec3_normalize(forward_pan_dir);
        vec3 right_pan_dir;
        glm_vec3_crossn(forward_pan_dir, WORLD_UP, right_pan_dir); // Right is same as yaw-only right


        // Calculate movement based on pan speeds
        vec3 move_vector = GLM_VEC3_ZERO_INIT;
        vec3 forward_move, right_move;

        // Forward/Backward panning (speed[1]) along XZ plane
        glm_vec3_scale(forward_pan_dir, camera->speed[1] * dt, forward_move);
        glm_vec3_add(move_vector, forward_move, move_vector);

        // Right/Left panning (speed[0]) along XZ plane
        glm_vec3_scale(right_pan_dir, camera->speed[0] * dt, right_move);
        glm_vec3_add(move_vector, right_move, move_vector);

        // --- Update BOTH target and position for panning ---
        // This makes the camera move parallel to the ground plane
        glm_vec3_add(camera->target_pos, move_vector, camera->target_pos);
        glm_vec3_add(camera->position, move_vector, camera->position);

    } else {
        // Ensure speed is zero if not allowed to pan
        // This prevents sliding if zoom changes while holding WASD
        glm_vec3_zero(camera->speed);
    }

    // Note: Position might also be updated by zoom_camera, ensure consistency
    // or always recalculate position from target/zoom/angles here?
    // Let's recalculate the position here to ensure it's always correct after updates.
    // calculate_camera_position(camera); // Recalculate position based on possibly updated target
    // This might fight panning - maybe only call if zoom changed?
    // Let's stick with updating both in the pan block for now.
}


void calculate_view_matrix(Camera* camera, mat4 dest) {
    if (!camera) return;
    // View matrix is determined by position, target, and world up
    // Position is now calculated based on target/zoom/angles or updated by panning
    glm_lookat(camera->position, camera->target_pos, WORLD_UP, dest);
}


void rotate_camera(Camera* camera, double horizontal, double vertical) {
    if (!camera) return;
    // Affect Yaw (rotation[1]) and Pitch (rotation[0])
    camera->rotation[1] += (float)horizontal * CAMERA_ROTATE_SENSITIVITY;
    camera->rotation[0] += (float)vertical * CAMERA_ROTATE_SENSITIVITY;

    // Wrap Yaw
    camera->rotation[1] = fmodf(camera->rotation[1], 360.0f);
    if (camera->rotation[1] < 0.0f) camera->rotation[1] += 360.0f;

    // Clamp Pitch
    if (camera->rotation[0] > CAMERA_PITCH_MAX) camera->rotation[0] = CAMERA_PITCH_MAX;
    if (camera->rotation[0] < CAMERA_PITCH_MIN) camera->rotation[0] = CAMERA_PITCH_MIN;

    // After rotating, recalculate the position based on new angles
    calculate_camera_position(camera);
}

void zoom_camera(Camera* camera, float amount) {
    if (!camera) return;
    camera->zoom_level -= amount * CAMERA_ZOOM_SENSITIVITY; // Subtract amount to zoom in

    // Clamp zoom level
    if (camera->zoom_level < camera->min_zoom) camera->zoom_level = camera->min_zoom;
    if (camera->zoom_level > camera->max_zoom) camera->zoom_level = camera->max_zoom;

    // After zooming, recalculate position based on new distance
    calculate_camera_position(camera);
}

// Combined setter for panning speed
void set_camera_pan_speed(Camera* camera, float forward_backward, float right_left) {
    if (!camera) return;
    // Only allow setting speed if the zoom level allows panning
    if (camera->zoom_level < CAMERA_PAN_THRESHOLD) {
        camera->speed[1] = forward_backward * CAMERA_PAN_SPEED; // Forward/Backward
        camera->speed[0] = right_left * CAMERA_PAN_SPEED;     // Right/Left Strafe
    } else {
        // Ensure the speed is zero if panning isn't allowed
        glm_vec3_zero(camera->speed);
    }
}

// show_texture_preview remains commented out / needs rewrite