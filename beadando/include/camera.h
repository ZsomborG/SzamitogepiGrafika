#ifndef CAMERA_H
#define CAMERA_H

#include <cglm/cglm.h>
#include <stdbool.h>
#include <glad/glad.h> // Keep for GLuint if needed elsewhere

struct Scene; // Forward declare

// --- Camera Constants ---
#define CAMERA_DEFAULT_YAW -90.0f // Look down -Z initially
#define CAMERA_DEFAULT_PITCH -30.0f // Look slightly down
#define CAMERA_DEFAULT_ZOOM 10.0f    // Initial distance from target
#define CAMERA_MIN_ZOOM 2.0f
#define CAMERA_MAX_ZOOM 15.0f
#define CAMERA_TARGET_Y_OFFSET 0.0f // Where the camera looks relative to board center Y
#define CAMERA_PAN_THRESHOLD 7.0f   // Zoom level below which panning is enabled

typedef struct Camera
{
    // Core Position & Orientation
    vec3 position;     // Calculated based on target, zoom, yaw, pitch
    vec3 rotation;     // Euler angles: [0]=Pitch, [1]=Yaw, [2]=Roll (Roll unused)

    // Look At Target (Center of interest, e.g., board center)
    vec3 target_pos;

    // Zoom Control
    float zoom_level;  // Distance from target_pos
    float min_zoom;
    float max_zoom;

    // Movement Speed (for panning when zoomed in)
    vec3 speed; // [0]=Right, [1]=Forward (relative to view), [2]=Up

    // --- Optional cached values ---
    // vec3 front; // Direction camera is facing
    // vec3 right;
    // vec3 up;
    // mat4 view_matrix;

    bool is_preview_visible; // Keep for texture preview if still needed

} Camera;

// --- Function Declarations ---

void init_camera(Camera* camera, vec3 initial_target);
void update_camera(Camera* camera, double time); // Renamed from calculate_derived_vectors or similar
void calculate_view_matrix(Camera* camera, mat4 dest);

// Input handling helpers
void rotate_camera(Camera* camera, double horizontal, double vertical);
void zoom_camera(Camera* camera, float amount); // New function for zoom adjustment
void set_camera_pan_speed(Camera* camera, float forward_backward, float right_left); // New combined function

// --- Remove old speed setters ---
// void set_camera_speed(Camera* camera, double speed);
// void set_camera_side_speed(Camera* camera, double speed);

#endif /* CAMERA_H */