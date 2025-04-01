#include "game.h"
#include "pong.h"

#include <GL/gl.h>
#include <stdio.h>
#include <math.h>

void init_game(Game* game, int width, int height)
{
    game->is_running = false;
    game->width = width;
    game->height = height;
    if (init_sdl(game) == false) {
        return;
    }
    init_opengl(game);
    init_pong(&(game->pong), width, height);
    update_score_title(game);

    printf("\n--- Pong Controls ---\n");
    printf("Player 1 (Left Paddle): W (Up), S (Down)\n");
    printf("Player 2 (Right Paddle): Mouse Y-axis\n");
    printf("Ball Size: UP Arrow (Increase), DOWN Arrow (Decrease)\n");
    printf("Move Ball: Left Mouse Click\n");
    printf("Quit: ESC\n");
    printf("----------------------\n\n");

    game->last_update_time = (double)SDL_GetTicks() / 1000;
    game->is_running = true;
}

void destroy_game(Game* game)
{
    if (game->gl_context != NULL) {
        SDL_GL_DeleteContext(game->gl_context);
    }

    if (game->window != NULL) {
        SDL_DestroyWindow(game->window);
    }

    SDL_Quit();
}

void handle_game_events(Game* game)
{
    SDL_Event event;
    int x;
    int y;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_KEYDOWN:
            switch (event.key.keysym.scancode) {
            case SDL_SCANCODE_ESCAPE:
                game->is_running = false;
                break;
            case SDL_SCANCODE_W:
                set_left_pad_speed(&(game->pong), -300);
                break;
            case SDL_SCANCODE_S:
                set_left_pad_speed(&(game->pong), +300);
                break;
            case SDL_SCANCODE_UP:
                game->pong.ball.radius += 2.0f;
                if (game->pong.ball.radius > MAX_BALL_RADIUS) 
                {
                    game->pong.ball.radius = MAX_BALL_RADIUS;
                }
                printf("Ball radius increased: %.1f\n", game->pong.ball.radius);
                break;
            case SDL_SCANCODE_DOWN:
                game->pong.ball.radius -= 2.0f;
                if (game->pong.ball.radius < MIN_BALL_RADIUS) 
                {
                    game->pong.ball.radius = MIN_BALL_RADIUS;
                }
                printf("Ball radius decreased: %.1f\n", game->pong.ball.radius);
                break;
            default:
                break;
            }
            break;

        case SDL_KEYUP:
            switch (event.key.keysym.scancode) {
            case SDL_SCANCODE_W:
            case SDL_SCANCODE_S:
                set_left_pad_speed(&(game->pong), 0);
                break;
            default:
                break;
            }
            break;

        case SDL_MOUSEMOTION:
            SDL_GetMouseState(&x, &y);
            set_right_pad_position(&(game->pong), y - game->pong.right_pad.height / 2.0f);
            break;

        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT) 
            {
                game->pong.ball.x = (float)event.button.x;
                game->pong.ball.y = (float)event.button.y;
                printf("Ball position set to (%d, %d)\n", event.button.x, event.button.y);
            }
            break;

        case SDL_QUIT:
            game->is_running = false;
            break;

        default:
            break;
        }
    }
}

void update_game(Game* game)
{
    double current_time;
    double elapsed_time;

    current_time = (double)SDL_GetTicks() / 1000;
    elapsed_time = current_time - game->last_update_time;

    game->last_update_time = current_time;

    update_pong(&(game->pong), elapsed_time, game);
}

void render_game(Game* game)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    render_pong(&(game->pong));
    SDL_GL_SwapWindow(game->window);
}

bool init_sdl(Game* game)
{
    int error_code;

    error_code = SDL_Init(SDL_INIT_EVERYTHING);
    if (error_code != 0) {
        printf("[ERROR] SDL initialization error: %s\n", SDL_GetError());
        return false;
    }

    game->window = SDL_CreateWindow(
        "Pong!",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        game->width, game->height,
        SDL_WINDOW_OPENGL);
    if (game->window == NULL) {
        printf("[ERROR] Unable to create the application window!\n");
        return false;
    }

    game->gl_context = SDL_GL_CreateContext(game->window);
    if (game->gl_context == NULL) {
        printf("[ERROR] Unable to create the OpenGL context!\n");
        return false;
    }

    return true;
}

void init_opengl(Game* game)
{
    glShadeModel(GL_SMOOTH);
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, (double)game->width, (double)game->height, 0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glViewport(0, 0, game->width, game->height);
}
