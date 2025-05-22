#ifndef IMGUI_INTERFACE_H
#define IMGUI_INTERFACE_H

// Include necessary types that the interface functions need,
// ensuring they are C-compatible.
#include <SDL2/SDL.h> // For SDL_Window, SDL_Event
#include "game_state.h"

#ifdef __cplusplus
extern "C" {
#endif


// --- Initialization and Shutdown ---

/**
 * @brief Initializes the ImGui context and backends.
 * Must be called after OpenGL context is created and GLAD is loaded.
 * @param window Pointer to the SDL_Window.
 * @param gl_context The SDL_GLContext.
 * @return true on success, false on failure.
 */
bool ImGui_InitWrapper(SDL_Window* window, SDL_GLContext gl_context);

/**
 * @brief Shuts down ImGui context and backends.
 */
void ImGui_ShutdownWrapper(void);


// --- Frame Handling ---

/**
 * @brief Starts a new ImGui frame. Call at the beginning of the render loop.
 */
void ImGui_NewFrameWrapper(void);

/**
 * @brief Ends the ImGui frame and prepares draw data. Call before rendering ImGui.
 */
void ImGui_RenderWrapper(void);

/**
 * @brief Renders the ImGui draw data. Call after ImGui_RenderWrapper().
 */
void ImGui_RenderDrawDataWrapper(void);


// --- Event Handling ---

/**
 * @brief Processes an SDL event for ImGui.
 * @param event Pointer to the SDL_Event.
 */
void ImGui_ProcessEventWrapper(const SDL_Event* event);

/**
 * @brief Checks if ImGui wants to capture mouse input.
 * @return true if ImGui wants the mouse, false otherwise.
 */
bool ImGui_WantCaptureMouseWrapper(void);

/**
 * @brief Checks if ImGui wants to capture keyboard input.
 * @return true if ImGui wants the keyboard, false otherwise.
 */
bool ImGui_WantCaptureKeyboardWrapper(void);


// --- Game UI Windows ---

/**
 * @brief Draws the Player Stats ImGui window.
 * Displays HP, Gold, Wave, Current Phase, and Mouse Hover Info.
 * @param gs Pointer to the current GameState.
 * @param mouse_is_over_board Is the mouse currently over a valid board tile.
 * @param hovered_grid_x The grid X coordinate the mouse is over (-1 if not over board).
 * @param hovered_grid_y The grid Y coordinate the mouse is over (-1 if not over board).
 */
void ImGui_DrawPlayerStatsWindowWrapper(const GameState* gs, bool mouse_is_over_board, int hovered_grid_x, int hovered_grid_y);

/**
 * @brief Draws the Shop ImGui window (placeholder).
 * Will contain unit purchase options later.
 * @param gs Pointer to the current GameState (needed for gold, etc.).
 * @return Bitmask indicating shop actions (e.g., bit 0 = refresh clicked,
 *         bit 1 = buy slot 0 clicked, etc. - for later). Returns 0 for now.
 */
int ImGui_DrawShopWindowWrapper(GameState* gs); // Pass non-const for gold deduction later

void ImGui_DrawBenchWindowWrapper(const struct Scene* scene, int* selected_bench_index_ptr, GamePhase current_phase);

/**
 * @brief Draws the Combat Info ImGui window (e.g., "Combat In Progress", progress bar).
 * @param gs Pointer to the current GameState (for timer).
 */
void ImGui_DrawCombatInfoWindowWrapper(const GameState* gs);

/**
 * @brief Draws the Post-Combat Summary ImGui window.
 * @param gs Pointer to the current GameState (for wave info).
 */
void ImGui_DrawPostCombatWindowWrapper(const GameState* gs);

/**
 * @brief Draws the Game Over ImGui window.
 * @param gs Pointer to the current GameState (for final wave).
 * @return true if "Restart" was clicked, false otherwise (for future use).
 */
bool ImGui_DrawGameOverWindowWrapper(const GameState* gs);

/**
 * @brief Draws the Help ImGui window.
 * Its visibility is controlled by the p_open boolean.
 * @param p_open Pointer to a boolean that controls the window's visibility.
 *               The window's own close button will set this to false.
 */
void ImGui_DrawHelpWindowWrapper(bool* p_open);

#ifdef __cplusplus
}  // extern "C"
#endif


#endif // IMGUI_INTERFACE_H