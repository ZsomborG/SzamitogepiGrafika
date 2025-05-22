// IMPORTANT: This is a C++ file (.cpp)

#include "imgui_interface.h" // Include the C header we just defined

// Include the actual ImGui headers ONLY in this C++ file
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl2.h>
#include <imgui/imgui_impl_opengl3.h> // Now using the actual header

#include <stdio.h> // For printf in example
#include "scene.h"
#include "unit.h"

#define ACTION_FLAG_NONE           0
#define ACTION_FLAG_REFRESH_SHOP   (1 << 0)
#define SHOP_ACTION_BUY_UNIT_TYPE_0     (1 << 1)
#define SHOP_ACTION_BUY_UNIT_TYPE_1     (1 << 2)
#define ACTION_FLAG_START_COMBAT   (1 << 10)

// Use extern "C" to ensure C linkage for the wrapper functions
extern "C" {

const float DISTANCE = 10.0f;
    
// --- Initialization and Shutdown ---

bool ImGui_InitWrapper(SDL_Window* window, SDL_GLContext gl_context) {
    printf("DEBUG: ImGui_InitWrapper - START\n");
    
    // First verify GL context is valid
    if (!gl_context) {
        printf("ERROR: ImGui_InitWrapper - Invalid GL context!\n");
        return false;
    }
    
    // Verify window is valid
    if (!window) {
        printf("ERROR: ImGui_InitWrapper - Invalid window!\n");
        return false;
    }
    
    printf("DEBUG: ImGui_InitWrapper - Creating ImGui context\n");
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    
    printf("DEBUG: ImGui_InitWrapper - Getting IO\n");
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    
    // Configure ImGui style
    printf("DEBUG: ImGui_InitWrapper - Setting style\n");
    ImGui::StyleColorsDark();

    // Initialize backends
    printf("DEBUG: ImGui_InitWrapper - Setting up backends...\n");
    const char* glsl_version = "#version 330 core";
    
    // Initialize SDL2 backend
    if (!ImGui_ImplSDL2_InitForOpenGL(window, gl_context)) {
        fprintf(stderr, "ERROR: ImGui_ImplSDL2_InitForOpenGL failed!\n");
        return false;
    }
    
    // Initialize OpenGL3 backend
    if (!ImGui_ImplOpenGL3_Init(glsl_version)) {
        fprintf(stderr, "ERROR: ImGui_ImplOpenGL3_Init failed!\n");
        ImGui_ImplSDL2_Shutdown();
        return false;
    }
    
    printf("DEBUG: ImGui_InitWrapper - ImGui Initialized.\n");
    return true;
}

void ImGui_ShutdownWrapper(void) {
    printf("DEBUG: ImGui_ShutdownWrapper - Shutting down ImGui...\n");
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    printf("DEBUG: ImGui_ShutdownWrapper - ImGui shutdown complete.\n");
}


// --- Frame Handling ---

void ImGui_NewFrameWrapper(void) {
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void ImGui_RenderWrapper(void) {
    // Prepares ImGui draw data
    ImGui::Render();
}

void ImGui_RenderDrawDataWrapper(void) {
    // Renders the ImGui draw data using the OpenGL backend
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


// --- Event Handling ---

void ImGui_ProcessEventWrapper(const SDL_Event* event) {
    if (!event) {
        return;
    }
    ImGui_ImplSDL2_ProcessEvent(event);
}

bool ImGui_WantCaptureMouseWrapper(void) {
    ImGuiIO& io = ImGui::GetIO();
    return io.WantCaptureMouse;
}

bool ImGui_WantCaptureKeyboardWrapper(void) {
    ImGuiIO& io = ImGui::GetIO();
    return io.WantCaptureKeyboard;
}

// --- Game UI Windows ---

void ImGui_DrawPlayerStatsWindowWrapper(const GameState* gs, bool is_mouse_over_board, int hovered_grid_x, int hovered_grid_y) {
    if (!gs) return;

    // --- Player Stats Window ---
    // Position it top-left, make it non-movable, non-resizable, transparent.
    const float DISTANCE = 10.0f;
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 window_pos(viewport->WorkPos.x + DISTANCE, viewport->WorkPos.y + DISTANCE);
    ImVec2 window_pos_pivot(0.0f, 0.0f); // Top-left pivot
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    ImGui::SetNextWindowBgAlpha(0.35f);

    if (ImGui::Begin("PlayerStats", NULL, window_flags)) {
        ImGui::Text("Wave: %d", gs->current_wave);
        ImGui::Separator();
        ImGui::Text("HP: %d", gs->player_hp);
        ImGui::Text("Gold: %d G", gs->player_gold); // Added "G"
        ImGui::Separator();
        ImGui::Text("Phase: %s", get_game_phase_name(gs->current_phase));
        ImGui::Separator(); // Add another separator

        // --- Add Mouse Hover Debug Info ---
        ImGui::Text("Mouse Over Board: %s", is_mouse_over_board ? "YES" : "NO");
        if (is_mouse_over_board) {
            ImGui::Text("Hovered Grid: (%d, %d)", hovered_grid_x, hovered_grid_y);
        } else {
            ImGui::Text("Hovered Grid: (N/A)");
        }
        // --- End Mouse Hover Debug Info ---
    }
    ImGui::End();
}


int ImGui_DrawShopWindowWrapper(GameState* gs) {
    if (!gs) return ACTION_FLAG_NONE;

    int action_flags = ACTION_FLAG_NONE; // Return value for actions

    // --- Shop Window ---
    // Position it bottom-center
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    // Center horizontally, place near bottom vertically
    const float bench_width_estimate = (BENCH_SIZE * 80.0f) + ((BENCH_SIZE) * ImGui::GetStyle().ItemSpacing.x) + ImGui::GetStyle().WindowPadding.x * 2.0f;
    const float shop_x_pos = viewport->WorkPos.x + DISTANCE + bench_width_estimate + DISTANCE; // Bench Pos + Bench Width + Spacing
    ImVec2 shop_window_pos(shop_x_pos, viewport->WorkPos.y + viewport->WorkSize.y - DISTANCE);
    ImVec2 shop_window_pos_pivot(0.0f, 1.0f);
    ImGui::SetNextWindowPos(shop_window_pos, ImGuiCond_Always, shop_window_pos_pivot);
    ImGui::SetNextWindowBgAlpha(0.65f);

    if (ImGui::Begin("Shop", NULL, window_flags)) {
        ImGui::Text("Shop (Gold: %d)", gs->player_gold);
        ImGui::Separator();

        if (gs->current_phase == PHASE_PREPARE) {
            ImGui::Text("Available Units:");
            ImGui::BeginGroup(); // Group units horizontally

            // --- Shop Slot 0: Tank ---
            UnitType type_slot0 = UNIT_MELEE_TANK;
            int cost_slot0 = get_unit_cost(type_slot0);
            char button_label_slot0[64];
            sprintf(button_label_slot0, "Buy %s [%dG]", get_unit_type_name(type_slot0), cost_slot0);

            bool can_afford_slot0 = (gs->player_gold >= cost_slot0); // Check affordability BEFORE button
            if (!can_afford_slot0) {
                ImGui::BeginDisabled();
            }
            if (ImGui::Button(button_label_slot0)) {
                if (can_afford_slot0) { // Only process click if it was affordable when drawn
                    // Gold is deducted in app.c, UI just signals intent
                    action_flags |= SHOP_ACTION_BUY_UNIT_TYPE_0;
                    printf("DEBUG: UI Clicked Buy %s (intended purchase).\n", get_unit_type_name(type_slot0));
                }
            }
            if (!can_afford_slot0) { // Use the SAME condition
                ImGui::EndDisabled();
            }
            ImGui::SameLine();

            // --- Shop Slot 1: Archer ---
            UnitType type_slot1 = UNIT_RANGED_ARCHER;
            int cost_slot1 = get_unit_cost(type_slot1);
            char button_label_slot1[64];
            sprintf(button_label_slot1, "Buy %s [%dG]", get_unit_type_name(type_slot1), cost_slot1);

            bool can_afford_slot1 = (gs->player_gold >= cost_slot1); // Check affordability
            if (!can_afford_slot1) {
                ImGui::BeginDisabled();
            }
            if (ImGui::Button(button_label_slot1)) {
                if (can_afford_slot1) {
                    action_flags |= SHOP_ACTION_BUY_UNIT_TYPE_1;
                    printf("DEBUG: UI Clicked Buy %s (intended purchase).\n", get_unit_type_name(type_slot1));
                }
            }
            if (!can_afford_slot1) { // Use the SAME condition
                ImGui::EndDisabled();
            }
            // ImGui::SameLine(); // If more slots

            ImGui::EndGroup();
            ImGui::Separator();

            // --- Other Shop Actions ---
            int refresh_cost = 1; // Example cost
            bool can_afford_refresh = (gs->player_gold >= refresh_cost);
            if (!can_afford_refresh) ImGui::BeginDisabled();
            if (ImGui::Button("Refresh [1G]")) { // Label should reflect actual cost
                if (can_afford_refresh) {
                    action_flags |= ACTION_FLAG_REFRESH_SHOP;
                    printf("DEBUG: UI Clicked Refresh Shop (intended action).\n");
                }
            }
            if (!can_afford_refresh) ImGui::EndDisabled();
            ImGui::SameLine();

            // Buy XP button (example)
            int xp_cost = 4;
            bool can_afford_xp = (gs->player_gold >= xp_cost);
            if (!can_afford_xp) ImGui::BeginDisabled();
            if (ImGui::Button("Buy XP [4G]")) {
                if(can_afford_xp){ /* TODO: Set action flag for buying XP */ }
            }
            if (!can_afford_xp) ImGui::EndDisabled();
            ImGui::SameLine();

            if (ImGui::Button("Lock Shop")) { /* TODO: Toggle lock state, set action flag */ }
            ImGui::Separator();

            if (ImGui::Button("Start Combat")) {
                action_flags |= ACTION_FLAG_START_COMBAT;
                printf("DEBUG: Start Combat button clicked in UI.\n");
            }

        } else {
            ImGui::Text("Shop closed during %s.", get_game_phase_name(gs->current_phase));
        }
    }
    ImGui::End();

    return action_flags;
}

const char* GetUnitTypeName(UnitType type) {
    switch(type) {
        case UNIT_MELEE_TANK: return "Tank";
        case UNIT_RANGED_ARCHER: return "Archer";
        default: return "Unknown";
    }
}

void ImGui_DrawBenchWindowWrapper(const struct Scene* scene, int* selected_bench_index_ptr, GamePhase current_phase) {
    if (!scene || !selected_bench_index_ptr) return;

    // --- Bench Window ---
    // Position it bottom-left, next to shop? Or along the whole bottom?
    // Let's place it to the left of the shop for now.
    const float DISTANCE = 10.0f; // Defined in PlayerStats, ensure consistency or pass as param/global
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 bench_window_pos(viewport->WorkPos.x + DISTANCE, viewport->WorkPos.y + viewport->WorkSize.y - DISTANCE);
    ImVec2 bench_window_pos_pivot(0.0f, 1.0f);
    ImGui::SetNextWindowPos(bench_window_pos, ImGuiCond_Always, bench_window_pos_pivot);
    ImGui::SetNextWindowBgAlpha(0.65f);


    if (ImGui::Begin("Bench", NULL, window_flags)) {
        ImGui::Text("Bench:");
        ImGui::Separator();

        int current_bench_count = 0;
        for (int i = 0; i < scene->unit_count; ++i) {
            if (scene->units[i].location == LOC_BENCH) {
                current_bench_count++;
                const Unit* bench_unit = &scene->units[i];
                char label[64];
                sprintf(label, "%s##Bench%d", GetUnitTypeName(bench_unit->type), bench_unit->id);
                bool is_selected = (*selected_bench_index_ptr == i);

                // --- Make selectable conditional on phase ---
                if (current_phase == PHASE_PREPARE) {
                    if (ImGui::Selectable(label, is_selected, 0, ImVec2(80, 50))) {
                        if (is_selected) { *selected_bench_index_ptr = -1; }
                        else { *selected_bench_index_ptr = i; }
                    }
                } else {
                    // Display as non-interactive text or disabled button if not prepare phase
                    ImGui::BeginDisabled();
                    ImGui::Button(GetUnitTypeName(bench_unit->type), ImVec2(80,50)); // Just display name
                    ImGui::EndDisabled();
                }
                // --- End Conditional Selectable ---

                // Tooltip (optional)
                if (ImGui::IsItemHovered()) {
                    ImGui::BeginTooltip();
                    ImGui::Text("Type: %s", GetUnitTypeName(bench_unit->type));
                    ImGui::Text("HP: %.0f", bench_unit->max_hp);
                    ImGui::Text("Damage: %.0f", bench_unit->attack_damage);
                    // Add other stats...
                    ImGui::EndTooltip();
                }


                // Arrange items horizontally
                if (current_bench_count < BENCH_SIZE) {
                    ImGui::SameLine();
                }
            }
        }

        // Fill remaining bench slots with placeholders (optional)
        for (int i = current_bench_count; i < BENCH_SIZE; ++i) {
            ImGui::BeginDisabled(); // Make placeholder non-interactive
            char label[32];
            sprintf(label, "Empty##BenchEmpty%d", i); // Unique ID
            ImGui::Button(label, ImVec2(80, 50));
            ImGui::EndDisabled();
            if (i < BENCH_SIZE - 1) {
                ImGui::SameLine();
            }
        }

        // Handle deselection if clicking outside selectables in the window? More complex.
        // If (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0) && !unit_selected_this_frame) {
        //    *selected_bench_index_ptr = -1;
        // }


    }
    ImGui::End();
}

void ImGui_DrawCombatInfoWindowWrapper(const GameState* gs) {
    if (!gs) return;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.1f), ImGuiCond_Always, ImVec2(0.5f, 0.0f)); // Top-center
    ImGui::SetNextWindowBgAlpha(0.50f);

    if (ImGui::Begin("CombatInfoOverlay", NULL, window_flags)) {
        ImGui::Text("COMBAT IN PROGRESS!");
        ImGui::ProgressBar(gs->combat_phase_timer / 5.0f, ImVec2(200.0f, 0.0f)); // Assuming 5s combat, fixed width
    }
    ImGui::End();
}

void ImGui_DrawPostCombatWindowWrapper(const GameState* gs) {
    if (!gs) return;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f)); // Centered
    ImGui::SetNextWindowBgAlpha(0.75f);

    if (ImGui::Begin("PostCombatSummary", NULL, window_flags)) {
        ImGui::Text("Round %d Over!", gs->current_wave -1); // Wave already incremented for next
        ImGui::Separator();
        ImGui::Text("Prepare for Wave %d", gs->current_wave);
        // Add more summary info later (gold earned, damage taken, etc.)
    }
    ImGui::End();
}

bool ImGui_DrawGameOverWindowWrapper(const GameState* gs) {
    if (!gs) return false;
    bool restart_clicked = false;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f)); // Centered
    ImGui::SetNextWindowBgAlpha(0.85f);

    if (ImGui::Begin("GameOverScreen", NULL, window_flags)) {
        ImGui::Text("GAME OVER");
        ImGui::Separator();
        ImGui::Text("You survived %d waves.", gs->current_wave -1); // Wave might be for the wave that killed player
        if (ImGui::Button("Restart? (Not Implemented yet)")) {
            restart_clicked = true;
            printf("DEBUG: Restart button clicked (Game Over screen).\n");
        }
    }
    ImGui::End();
    return restart_clicked;
}

void ImGui_DrawHelpWindowWrapper(bool* p_open) {
    if (!p_open || !(*p_open)) { // Don't even begin if p_open is NULL or false
        return;
    }

    // Begin a new window. Passing p_open makes the window have a close button
    // that will set *p_open to false when clicked.
    if (ImGui::Begin("Help Guide (F1 to toggle)", p_open, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Welcome to AutoChess Game!");
        ImGui::Separator();

        ImGui::TextWrapped("This is a simplified auto-battler game. The goal is to survive waves of AI enemies by strategically buying, placing, and (eventually) upgrading units.");
        ImGui::Spacing();

        ImGui::CollapsingHeader("Game Phases:");
        ImGui::Indent();
        ImGui::Text("1. Prepare Phase: Buy units from the Shop, place them on your side of the board.");
        ImGui::Text("2. Combat Phase: Click 'Start Combat'. Your units will automatically fight the AI wave.");
        ImGui::Text("3. Post-Combat: Results are shown, you get gold for the next round. AI gets stronger.");
        ImGui::Unindent();
        ImGui::Spacing();

        ImGui::CollapsingHeader("Controls:");
        ImGui::Indent();
        ImGui::Text("Mouse Wheel: Zoom camera in/out.");
        ImGui::Text("Right-Click + Drag Mouse: Rotate camera.");
        ImGui::Text("W, A, S, D: Pan camera (when zoomed in close enough).");
        ImGui::Text("Left-Click (Shop): Buy units.");
        ImGui::Text("Left-Click (Bench): Select a unit for placement.");
        ImGui::Text("Left-Click (Board - Player Side): Place selected unit if tile is valid.");
        ImGui::Text("F1: Toggle this Help window.");
        ImGui::Unindent();
        ImGui::Spacing();

        ImGui::CollapsingHeader("Unit Types (Example Stats):");
        ImGui::Indent();
        ImGui::Text("Tank (Melee):");
        ImGui::BulletText("HP: 100, Damage: 10, Attack Speed: 0.8, Range: Short");
        ImGui::Text("Archer (Ranged):");
        ImGui::BulletText("HP: 60, Damage: 15, Attack Speed: 1.1, Range: Long");
        ImGui::Unindent();
        ImGui::Spacing();

        if (ImGui::Button("Close")) {
            *p_open = false; // Also allow closing with a button
        }
    }
    ImGui::End();
}

} // extern "C"