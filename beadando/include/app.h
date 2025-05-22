#ifndef APP_H
#define APP_H

#include "camera.h"
#include "scene.h"
#include "game_state.h"
#include "input.h"

#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <cglm/cglm.h>

#include <stdbool.h>

#define VIEWPORT_RATIO (4.0 / 3.0)
#define VIEWPORT_ASPECT 50.0

typedef struct App
{
    SDL_Window* window;
    SDL_GLContext gl_context;
    bool is_running;
    double uptime;
    
    Camera camera;
    Scene scene;
    GameState game_state;
    InputState input_state;
    
    GLuint shader_program;
    mat4 projection_matrix;
    mat4 view_matrix;
    
    GLint shader_uloc_projection;
    GLint shader_uloc_view;
    GLint shader_uloc_model;
    GLint shader_uloc_texture1;
    GLint shader_uloc_color_tint;
    
    int selected_bench_unit_index;
    
    bool show_help_window;
    
    // Lighting
    vec3 light_direction_world;
    vec3 light_color;
    vec3 ambient_light_color;

    
    GLint shader_uloc_lightDir;
    GLint shader_uloc_lightColor;
    GLint shader_uloc_ambientLightColor;
    GLint shader_uloc_viewPos;
    GLint shader_uloc_materialDiffuse;
    GLint shader_uloc_materialSpecular;
    GLint shader_uloc_materialShininess;
} App;

/**
 * Initialize the application.
 */
void init_app(App* app, int width, int height);

/**
 * Initialize the OpenGL context.
 */
void init_opengl();

/**
 * Reshape the window.
 */
void reshape(GLsizei width, GLsizei height);

/**
 * Update the application.
 */
void update_app(App* app);

/**
 * Render the application.
 */
void render_app(App* app);

/**
 * Destroy the application.
 */
void destroy_app(App* app);

#endif /* APP_H */
