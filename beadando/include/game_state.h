#ifndef GAME_STATE_H
#define GAME_STATE_H

#ifdef __cplusplus
extern "C" {
#endif


typedef enum GamePhase {
    PHASE_PREPARE,
    PHASE_COMBAT,
    PHASE_POST_COMBAT,
    PHASE_GAME_OVER
} GamePhase;

typedef struct GameState {
    GamePhase current_phase;
    
    int player_hp;
    int player_gold;
    int current_wave;
    
    float combat_phase_timer;
} GameState;

/**
 * @brief Converts a GamePhase enum value to a string representation.
 * @param phase The GamePhase enum value.
 * @return A constant string representing the phase name.
 */
const char* get_game_phase_name(GamePhase phase);

#ifdef __cplusplus
}
#endif

#endif //GAME_STATE_H
