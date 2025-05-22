#ifndef INPUT_H
#define INPUT_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <cglm/cglm.h> // For vec3

// Forward declare App struct if InputManager needs access to it (e.g., for matrices)
// However, it's often cleaner to pass necessary data directly to functions.
struct App; // Forward declaration of App

// --- Input State Structure ---
// This structure can hold the processed input state for the current frame.
typedef struct InputState {
    // Mouse
    int mouse_screen_x;
    int mouse_screen_y;
    bool left_mouse_pressed;    // True if pressed THIS frame
    bool left_mouse_down;       // True if currently held down
    bool left_mouse_released;   // True if released THIS frame
    bool right_mouse_pressed;
    bool right_mouse_down;
    bool right_mouse_released;
    float mouse_wheel_delta_y;   // Scroll amount this frame

    // Mouse picking results
    int hovered_grid_x;
    int hovered_grid_y;
    bool is_mouse_over_board;
    vec3 mouse_world_pos_on_plane; // Intersection point on the board plane

    // Keyboard (for specific actions, continuous movement handled by GetKeyboardState directly)
    bool quit_requested;

    // Camera Panning intent (derived from keyboard state)
    float pan_forward_backward_intent; // -1.0 (S), 0.0, 1.0 (W)
    float pan_right_left_intent;     // -1.0 (A), 0.0, 1.0 (D)

    // ImGui focus
    bool imgui_wants_mouse;
    bool imgui_wants_keyboard;
    
    bool f1_pressed_this_frame;

    bool plus_key_pressed;
    bool minus_key_pressed;
} InputState;


// --- Input Manager Functions ---

/**
 * @brief Initializes any internal state for the input manager.
 * (Might not be needed if all state is in InputState and passed around)
 */
// void InputManager_Init(void); // Example if internal state was needed

/**
 * @brief Polls SDL events, processes ImGui events, and updates the InputState struct.
 * This should be called once per frame at the beginning of the game loop.
 * @param app Pointer to the main App struct (needed for matrices, camera for ray casting).
 * @param input_state Pointer to an InputState struct to be filled.
 */
void InputManager_PollAndProcess(struct App* app, InputState* input_state);

/**
 * @brief Shuts down or cleans up any input manager resources.
 * (Might not be needed for this simple version)
 */
// void InputManager_Shutdown(void);

#endif // INPUT_H