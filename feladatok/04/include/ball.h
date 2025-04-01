#ifndef BALL_H
#define BALL_H

#define MIN_BALL_RADIUS 10.0f
#define MAX_BALL_RADIUS 80.0f
#define INITIAL_ROTATION_SPEED 90.0f

/**
 * Ball position and speed
 */
typedef struct Ball
{
    float x;
    float y;
    float radius;
    float speed_x;
    float speed_y;
    float angle;
    float rotation_speed;
} Ball;

/**
 * Move the ball to the given position.
 */
void init_ball(Ball* ball, float x, float y);

/**
 * Update the ball.
 * @param time elapsed time in seconds.
 */
void update_ball(Ball* ball, double time);

/**
 * Render the ball.
 */
void render_ball(Ball* ball);

#endif /* BALL_H */
