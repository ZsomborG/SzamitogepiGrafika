#include "scene.h"
#include "utils.h"

#include <GL/gl.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void init_scene(Scene* scene)
{
    scene->sphere_rotation_angle = 0.0f;
}

void update_scene(Scene* scene, double time)
{
    scene->sphere_rotation_angle += 45.0f * (float)time;
    if (scene->sphere_rotation_angle > 360.0f) 
    {
        scene->sphere_rotation_angle -= 360.0f;    
    }
}

void draw_sphere(float radius, int slices, int stacks)
{
    glColor3f(0.6f, 0.6f, 0.6f);

    for (int i = 0; i < stacks; ++i) 
    {
        float phi1 = M_PI * (float)i / stacks - M_PI / 2.0f;
        float phi2 = M_PI * (float)(i + 1) / stacks - M_PI / 2.0f;

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= slices; ++j)
        {
            float theta = 2.0f * M_PI * (float)j / slices;

            float x2 = radius * cos(phi2) * cos(theta);
            float y2 = radius * cos(phi2) * sin(theta);
            float z2 = radius * sin(phi2);
            glNormal3f(x2 / radius, y2 / radius, z2 / radius);
            glVertex3f(x2, y2, z2);

            float x1 = radius * cos(phi1) * cos(theta);
            float y1 = radius * cos(phi1) * sin(theta);
            float z1 = radius * sin(phi1);
            glNormal3f(x1 / radius, y1 / radius, z1 / radius);
            glVertex3f(x1, y1, z1);
        }
        glEnd();
    }
}

void draw_checkerboard(int width_squares, int height_squares, float square_size)
{
    glNormal3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_QUADS);
    for (int i = 0; i < width_squares; ++i) 
    {
        for (int j = 0; j < height_squares; ++j)
        {
            if ((i + j) % 2 == 0)
            {
                glColor3f(0.9f, 0.9f, 0.9f);
            }
            else
            {
                glColor3f(0.1f, 0.1f, 0.1f);
            }

            float x0 = i * square_size;
            float y0 = j * square_size;
            float x1 = (i + 1) * square_size;
            float y1 = (j + 1) * square_size;

            glVertex3f(x0, y0, 0.0f);
            glVertex3f(x1, y0, 0.0f);
            glVertex3f(x1, y1, 0.0f);
            glVertex3f(x0, y1, 0.0f);
        }
    }
    glEnd();
}

void draw_cylinder(float radius, float height, int segments)
{
    float angle_step = 2.0f * M_PI / segments;

    glColor3f(0.0f, 0.3f, 0.8f);
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= segments; ++i)
    {
        float angle = angle_step * i;
        float x = cos(angle) * radius;
        float y = sin(angle) * radius;
        float nx = cos(angle);
        float ny = sin(angle);

        glNormal3f(nx, ny, 0.0f);
        glVertex3f(x, y, 0.0f);

        glNormal3f(nx, ny, 0.0f);
        glVertex3f(x, y, height);
    }
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    for (int i = 0; i <= segments; ++i)
    {
        float angle = angle_step * i;
        float x = cos(angle) * radius;
        float y = sin(angle) * radius;
        glVertex3f(x, y, 0.0f);
    }
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, height);
    for (int i = 0; i <= segments; ++i)
    {
        float angle = angle_step * i;
        float x = cos(angle) * radius;
        float y = sin(angle) * radius;
        glVertex3f(x, y, height);
    }
    glEnd();
}

void draw_cone(float radius, float height, int segments)
{
    float angle_step = 2.0f * M_PI / segments;

    glColor3f(0.8f, 0.7f, 0.0f);
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, height);
    for (int i = 0; i <= segments; ++i)
    {
        float angle = angle_step * i;
        float x = cos(angle) * radius;
        float y = sin(angle) * radius;

        float nx = cos(angle);
        float ny = sin(angle);
        float nz = radius / height;
        vec3 normal = vec3_normalize((vec3){nx, ny, nz});

        glNormal3f(normal.x, normal.y, normal.z);
        glVertex3f(x, y, 0.0f);
    }
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    for (int i = 0; i <= segments; ++i)
    {
        float angle = angle_step * i;
        float x = cos(angle) * radius;
        float y = sin(angle) * radius;

        glVertex3f(x, y, 0.0f);
    }
    glEnd();
}

void render_scene(const Scene* scene)
{
    GLboolean lightning_enabled = GL_FALSE;
    glGetBooleanv(GL_LIGHTING, &lightning_enabled);
    if (lightning_enabled)
    {
        glDisable(GL_LIGHTING);
    }

    draw_origin();

    if (lightning_enabled)
    {
        glEnable(GL_LIGHTING);
    }

    glPushMatrix();
        glTranslatef(0.0f, 0.0f, -0.001f);
        draw_checkerboard(10, 10, 0.1f);
    glPopMatrix();

    if (lightning_enabled)
    {
        glEnable(GL_LIGHTING);
    }   
    glBegin(GL_TRIANGLES);
        glColor3f(1.0f, 1.0f, 0.0f);
        glVertex3f(0.2f, 0.2f, 0.1f);

        glColor3f(0.0f, 1.0f, 1.0f);
        glVertex3f(0.8f, 0.2f, 0.1f);

        glColor3f(1.0f, 0.0f, 1.0f);
        glVertex3f(0.5f, 0.8f, 0.1f);
    glEnd();

    if (lightning_enabled)
    {
        glEnable(GL_LIGHTING);
    }

    glPushMatrix();
        glTranslatef(0.3f, 0.7f, 0.3f);
        glRotatef(scene->sphere_rotation_angle, 0.0f, 1.0f, 0.0f);
        draw_sphere(0.15f, 12, 12);
    glPopMatrix();


    glPushMatrix();
        glTranslatef(0.7f, 0.7f, 0.0f);
        draw_cylinder(0.1f, 0.5f, 16);
    glPopMatrix();


    glPushMatrix();
        glTranslatef(0.7f, 0.3f, 0.0f);
        draw_cone(0.15f, 0.4f, 16);
    glPopMatrix();


}

void draw_origin()
{
    glLineWidth(2.0f);
    glBegin(GL_LINES);

    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(1.0f, 0.0f, 0.0f);

    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 1.0f, 0.0f);

    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 1.0f);

    glEnd();
}
