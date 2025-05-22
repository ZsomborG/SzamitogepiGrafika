#include "game_state.h"

const char* get_game_phase_name(GamePhase phase) {
    switch (phase) {
        case PHASE_PREPARE:     return "Prepare Phase";
        case PHASE_COMBAT:      return "Combat Phase";
        case PHASE_POST_COMBAT: return "Round End Phase";
        case PHASE_GAME_OVER:   return "Game Over";
        default:                return "Unknown Phase";
    }
}