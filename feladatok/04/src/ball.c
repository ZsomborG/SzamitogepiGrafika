#include "ball.h"

#include <GL/gl.h>

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


void init_ball(Ball* ball, float x, float y)
{
    ball->x = x;
    ball->y = y;
    ball->radius = 30;
    ball->speed_x = 250;
    ball->speed_y = 250;
    ball->angle = 0.0f;
    ball->rotation_speed = INITIAL_ROTATION_SPEED;
}

void update_ball(Ball* ball, double time)
{
    ball->x += ball->speed_x * time;
    ball->y += ball->speed_y * time;

    ball->angle += ball->rotation_speed * time;
    
    ball->angle = fmodf(ball->angle, 360.0f);
    if (ball->angle < 0.0f) {
        ball->angle += 360.0f;
    }
}

void render_ball(Ball* ball)
{
    double angle_rad;
    double x_vtx;
    double y_vtx;

    glPushMatrix();
    glTranslatef(ball->x, ball->y, 0.0);
    glRotatef(ball->angle, 0.0, 0.0, 1.0);
    glBegin(GL_TRIANGLE_FAN);
    glColor3f(0.9f, 0.9f, 0.9f);
    glVertex2f(0, 0);
    
    int num_segments = 24;
    for (int i = 0; i < num_segments; i++) {
        angle_rad = (double)i / (double)num_segments * 2.0 * M_PI;
        x_vtx = cos(angle_rad) * ball->radius;
        y_vtx = sin(angle_rad) * ball->radius;
        
        float r = 0.8f + 0.2f * (float)cos(angle_rad + M_PI / 4.0);
        float g = 0.7f + 0.2f * (float)sin(angle_rad + M_PI / 4.0);
        float b = 0.6f;
        glColor3f(r, g, b);
        glVertex2f(x_vtx, y_vtx);
    }
    glEnd();

    glBegin(GL_LINES);
    glColor3f(0.1f, 0.1f, 0.1f);
    glVertex2f(0, 0);
    glVertex2f(ball->radius * 0.9f, 0);
    glEnd();
    glPopMatrix();
}
