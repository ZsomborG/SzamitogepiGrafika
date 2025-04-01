#include "pong.h"
#include "game.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <SDL2/SDL.h>

void reset_ball(Pong* pong)
{
    pong->ball.x = pong->width / 2.0f;
    pong->ball.y = pong->height / 2.0f;

    float speed_magnitute = sqrt(pong->ball.speed_x * pong->ball.speed_x + pong->ball.speed_y * pong->ball.speed_y);
    if(speed_magnitute < 50.f || speed_magnitute > 1000.f) 
    {
        speed_magnitute = 250.f;
    }

    int direction = (rand() % 2 == 0) ? 1 : -1;
    pong->ball.speed_x = direction * speed_magnitute * 0.7f;

    pong->ball.speed_y = ((rand() % 100) / 100.0f - 0.5f) * speed_magnitute * 0.6f;
    if(fabs(pong->ball.speed_y) < speed_magnitute * 0.1f)
    {
        pong->ball.speed_y = (rand() % 2 == 0) ? speed_magnitute * 0.2f : -speed_magnitute * 0.2f;
    }

    pong->ball.rotation_speed = INITIAL_ROTATION_SPEED * ((rand() % 2 == 0) ? 1.0f : -1.0f);
}

void update_score_title(Game* game)
{
    if(!game || !game->window) return;
    char title[100];
    snprintf(title, sizeof(title), "Pong! | Player 1 (Red): %d | Player 2 (Green): %d", game->pong.score_left, game->pong.score_right);
    SDL_SetWindowTitle(game->window, title);
}

void init_pong(Pong* pong, int width, int height)
{
    pong->width = width;
    pong->height = height;
    init_pad(&(pong->left_pad), 0, height, RED_THEME);
    init_pad(&(pong->right_pad), width - 50.0f, height, GREEN_THEME);
    init_ball(&(pong->ball), width / 2.0f, height / 2.0f);
    pong->score_left = 0;
    pong->score_right = 0;
    srand((unsigned int)time(NULL));
}

void update_pong(Pong* pong, double time, Game* game)
{
    update_pad(&(pong->left_pad), time);
    update_pad(&(pong->right_pad), time);
    update_ball(&(pong->ball), time);
    bounce_ball(pong, game);
}

void render_pong(Pong* pong)
{
    render_pad(&(pong->left_pad));
    render_pad(&(pong->right_pad));
    render_ball(&(pong->ball));
}

void set_left_pad_position(Pong* pong, float position)
{
    set_pad_position(&(pong->left_pad), position);
}

void set_left_pad_speed(Pong* pong, float speed)
{
    set_pad_speed(&(pong->left_pad), speed);
}

void set_right_pad_position(Pong* pong, float position)
{
    set_pad_position(&(pong->right_pad), position);
}

void set_right_pad_speed(Pong* pong, float speed)
{
    set_pad_speed(&(pong->right_pad), speed);
}

void bounce_ball(Pong* pong, Game* game)
{
    if (pong->ball.y - pong->ball.radius < 0.0f) 
    {
        pong->ball.y = pong->ball.radius;
        pong->ball.speed_y *= -1.0;
        pong->ball.rotation_speed *= 0.95;
    }

    if (pong->ball.y + pong->ball.radius > pong->height) 
    {
        pong->ball.y = pong->height - pong->ball.radius;
        pong->ball.speed_y *= -1.0;
        pong->ball.rotation_speed *= 0.95;
    }

    if (pong->ball.speed_x < 0.0f && pong->ball.x - pong->ball.radius <= pong->left_pad.x + pong->left_pad.width)
    {
        bool hit_left_pad = (pong->ball.y + pong->ball.radius > pong->left_pad.y) &&
                            (pong->ball.y - pong->ball.radius < pong->left_pad.y + pong->left_pad.height);

        if (hit_left_pad)
        {
            pong->ball.x = pong->left_pad.x + pong->left_pad.width + pong->ball.radius;
            pong->ball.speed_x *= -1.05;

            float hit_center_offset = (pong->ball.y - (pong->left_pad.y + pong->left_pad.height / 2.0f)) / (pong->left_pad.height / 2.0f);
            pong->ball.speed_y += hit_center_offset * 150.f;

            pong->ball.rotation_speed *= -1.1f;
            pong->ball.rotation_speed += hit_center_offset * 60.0f;

            float max_speed = 800.0f;
            float current_speed = sqrtf(pong->ball.speed_x * pong->ball.speed_x + pong->ball.speed_y * pong->ball.speed_y);
            if (current_speed > max_speed)
            {
                pong->ball.speed_x *= (pong->ball.speed_x / current_speed) * max_speed;
                pong->ball.speed_y *= (pong->ball.speed_y / current_speed) * max_speed;
            }
        }
    }

    if (pong->ball.speed_x > 0.0f && pong->ball.x + pong->ball.radius >= pong->right_pad.x)
    {
        bool hit_right_pad = (pong->ball.y + pong->ball.radius > pong->right_pad.y) &&
                             (pong->ball.y - pong->ball.radius < pong->right_pad.y + pong->right_pad.height);

        if (hit_right_pad)
        {
            pong->ball.x = pong->right_pad.x - pong->ball.radius;
            pong->ball.speed_x *= -1.05;

            float hit_center_offset = (pong->ball.y - (pong->right_pad.y + pong->right_pad.height / 2.0f)) / (pong->right_pad.height / 2.0f);
            pong->ball.speed_y += hit_center_offset * 150.f;

            pong->ball.rotation_speed *= -1.1f;
            pong->ball.rotation_speed += hit_center_offset * 60.0f;

            float max_speed = 800.0f;
            float current_speed = sqrtf(pong->ball.speed_x * pong->ball.speed_x + pong->ball.speed_y * pong->ball.speed_y);
            if (current_speed > max_speed)
            {
                pong->ball.speed_x *= (pong->ball.speed_x / current_speed) * max_speed;
                pong->ball.speed_y *= (pong->ball.speed_y / current_speed) * max_speed;
            }
        }
    }

    bool scored = false;
    if (pong->ball.x - pong->ball.radius < 0.0f) 
    {
        pong->score_right++;
        printf("Player 2 scored!\nScore: %d - %d\n", pong->score_left, pong->score_right);
        scored = true;
    }

    else if (pong->ball.x + pong->ball.radius > pong->width) 
    {
        pong->score_left++;
        printf("Player 1 scored!\nScore: %d - %d\n", pong->score_left, pong->score_right);
        scored = true;
    }

    if (scored) 
    {
        reset_ball(pong);
        update_score_title(game);
    }
}
