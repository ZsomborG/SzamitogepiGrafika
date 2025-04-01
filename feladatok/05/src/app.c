#include "app.h"

#include <SDL2/SDL_image.h>
#include <stdio.h>


void init_app(App* app, int width, int height)
{
    int error_code;
    int inited_loaders;

    app->is_running = false;

    error_code = SDL_Init(SDL_INIT_EVERYTHING);
    if (error_code != 0) {
        printf("[ERROR] SDL initialization error: %s\n", SDL_GetError());
        return;
    }

    app->window = SDL_CreateWindow(
        "Origin Modifications",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_OPENGL);
    if (app->window == NULL) {
        printf("[ERROR] Unable to create the application window!\n");
        SDL_Quit();
        return;
    }

    inited_loaders = IMG_Init(IMG_INIT_PNG);
    if ((inited_loaders & IMG_INIT_PNG) == 0) {
        printf("[ERROR] IMG initialization error (PNG not supported): %s\n", IMG_GetError());
        SDL_DestroyWindow(app->window);
        SDL_Quit();
        return;
    }

    app->gl_context = SDL_GL_CreateContext(app->window);
    if (app->gl_context == NULL) {
        printf("[ERROR] Unable to create the OpenGL context!\n");
        IMG_Quit();
        SDL_DestroyWindow(app->window);
        SDL_Quit();
        return;
    }

    init_opengl();
    reshape(width, height);

    init_camera(&(app->camera));
    init_scene(&(app->scene));

    app->uptime = (double)SDL_GetTicks() / 1000.0;
    app->is_running = true;
}

void init_opengl()
{
    glShadeModel(GL_SMOOTH);

    glEnable(GL_NORMALIZE);
    glEnable(GL_AUTO_NORMAL);

    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_DEPTH_TEST);
    glClearDepth(1.0);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    GLfloat light_pos[] = { 1.0f, 1.0f, 3.0f, 1.0f };
    GLfloat light_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    glLineWidth(2.0f);
}

void reshape(GLsizei width, GLsizei height)
{
    int x, y, w, h;
    double ratio;

    ratio = (double)width / height;
    if (ratio > VIEWPORT_RATIO) {
        w = (int)((double)height * VIEWPORT_RATIO);
        h = height;
        x = (width - w) / 2;
        y = 0;
    }
    else {
        w = width;
        h = (int)((double)width / VIEWPORT_RATIO);
        x = 0;
        y = (height - h) / 2;
    }

    glViewport(x, y, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(
        -.08 * VIEWPORT_RATIO, .08 * VIEWPORT_RATIO,
        -.06, .08,
        0.1, 1000.0
    );

    glMatrixMode(GL_MODELVIEW);
}

void handle_app_events(App* app)
{
    SDL_Event event;
    static bool is_mouse_down = false;
    static int mouse_x = 0;
    static int mouse_y = 0;
    int x;
    int y;
    const double MOUSE_SENSITIVITY = 0.1;
    const double KEY_ROTATE_SPEED = 2.0;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_KEYDOWN:
            switch (event.key.keysym.scancode) {
            case SDL_SCANCODE_ESCAPE:
                app->is_running = false;
                break;
            case SDL_SCANCODE_W:
                set_camera_speed(&(app->camera), 1.5);
                break;
            case SDL_SCANCODE_S:
                set_camera_speed(&(app->camera), -1.5);
                break;
            case SDL_SCANCODE_D:
                set_camera_side_speed(&(app->camera), 1.5);
                break;
            case SDL_SCANCODE_A:
                set_camera_side_speed(&(app->camera), -1.5);
                break;
            case SDL_SCANCODE_J:
                rotate_camera(&(app->camera), KEY_ROTATE_SPEED, 0.0);
                break;
            case SDL_SCANCODE_L:
                rotate_camera(&(app->camera), -KEY_ROTATE_SPEED, 0.0);
                break;
            case SDL_SCANCODE_Q:
                rotate_camera(&(app->camera), 0.0, KEY_ROTATE_SPEED);
                break;
            case SDL_SCANCODE_E:
                rotate_camera(&(app->camera), 0.0, -KEY_ROTATE_SPEED);
                break;
            default:
                break;
            }
            break;
        case SDL_KEYUP:
            switch (event.key.keysym.scancode) {
            case SDL_SCANCODE_W:
            case SDL_SCANCODE_S:
                set_camera_speed(&(app->camera), 0);
                break;
            case SDL_SCANCODE_A:
            case SDL_SCANCODE_D:
                set_camera_side_speed(&(app->camera), 0);
                break;
            default:
                break;
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT) {
                is_mouse_down = true;
            }
            break;
        case SDL_MOUSEMOTION:
            SDL_GetMouseState(&x, &y);
            if (is_mouse_down) {
                float horizontal_rotation = (mouse_x - x) * MOUSE_SENSITIVITY;
                float vertical_rotation = -(mouse_y - y) * MOUSE_SENSITIVITY;
                rotate_camera(&(app->camera), horizontal_rotation, vertical_rotation);
            }
            mouse_x = x;
            mouse_y = y;
            break;
        case SDL_MOUSEBUTTONUP:
            if (event.button.button == SDL_BUTTON_LEFT) {
                is_mouse_down = false;
            }
            break;
        case SDL_QUIT:
            app->is_running = false;
            break;
        default:
            break;
        }
    }
}

void update_app(App* app)
{
    double current_time;
    double elapsed_time;

    current_time = (double)SDL_GetTicks() / 1000;
    elapsed_time = current_time - app->uptime;
    app->uptime = current_time;

    update_camera(&(app->camera), elapsed_time);
    update_scene(&(app->scene), elapsed_time);
}

void render_app(App* app)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);

    glPushMatrix();
    set_view(&(app->camera));
    render_scene(&(app->scene));
    glPopMatrix();

    SDL_GL_SwapWindow(app->window);
}

void destroy_app(App* app)
{
    if (app->gl_context != NULL) {
        SDL_GL_DeleteContext(app->gl_context);
    }

    if (app->window != NULL) {
        SDL_DestroyWindow(app->window);
    }

    SDL_Quit();
}
