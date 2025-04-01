#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "circle.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define MAX_LINES 100
#define MAX_CIRCLES 50
#define PALETTE_HEIGHT 50
#define PALETTE_BOX_WIDTH 40


typedef struct {
    int x1, y1;
    int x2, y2;
    Color color;
} Line;

typedef struct {
    int x, y, w, h;
    Color color;
} Rectangle;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;

Line gLines[MAX_LINES];
int gLineCount = 0;
bool gIsDrawingLine = false;
int gLineStartX, gLineStartY;

Rectangle gRectangles[MAX_LINES];
int gRectangleCount = 0;
bool gIsDrawingRect = false;
int gRectStartX, gRectStartY;

Circle gCircles[MAX_CIRCLES];
int gCircleCount = 0;
int gDraggingCircleIndex = -1;
int gDragOffsetX, gDragOffsetY;

Color gPaletteColors[] = {
    {255, 0,   0,   255}, // Red
    {0,   255, 0,   255}, // Green
    {0,   0,   255, 255}, // Blue
    {255, 255, 0,   255}, // Yellow
    {0,   255, 255, 255}, // Cyan
    {255, 0,   255, 255}, // Magenta
    {255, 255, 255, 255}, // White
    {0,   0,   0,   255}  // Black
};
int gPaletteColorCount = sizeof(gPaletteColors) / sizeof(gPaletteColors[0]);

Color gCurrentColor = {255, 255, 255, 255}; // Default: White

typedef enum {
    MODE_DRAW_LINE,
    MODE_DRAW_RECT,
    MODE_DRAW_CIRCLE,
    MODE_SELECT_DRAG
} DrawMode;

DrawMode gCurrentMode = MODE_DRAW_LINE;

bool initSDL();
void closeSDL();
void handleEvents(SDL_Event* e, bool* quit);
void render();
void drawPalette();
void drawLines();
void drawRectangles();
void drawCircleApprox(SDL_Renderer* renderer, int centerX, int centerY, int radius, Color color, int num_segments);
void drawCircles();
void drawCircleHighlight(const Circle* circle);
void drawCircleApproxAngleStep(SDL_Renderer* renderer, int centerX, int centerY, int radius, Color color, double angleStepRad);
void drawCircleApproxMaxLen(SDL_Renderer* renderer, int centerX, int centerY, int radius, Color color, double maxSegmentLen);

int main(int argc, char* argv[]) 
{
    if (!initSDL()) {
        printf("Failed to initialize SDL!\n");
        return -1;
    }

    bool quit = false;
    SDL_Event e;

    if (gCircleCount < MAX_CIRCLES) {
         set_circle_data(&gCircles[gCircleCount], 150, 200, 50);
         set_circle_color(&gCircles[gCircleCount], (Color){0, 0, 255, 255});
         gCircleCount++;
    }
     if (gCircleCount < MAX_CIRCLES) {
         set_circle_data(&gCircles[gCircleCount], 400, 300, 80);
         set_circle_color(&gCircles[gCircleCount], (Color){255, 0, 0, 255});
         gCircleCount++;
    }
     if (gCircleCount < MAX_CIRCLES) {
         set_circle_data(&gCircles[gCircleCount], 600, 150, 30);
         set_circle_color(&gCircles[gCircleCount], (Color){0, 255, 0, 255});
         gCircleCount++;
    }


    while (!quit) 
    {
        while (SDL_PollEvent(&e) != 0) 
        {
            handleEvents(&e, &quit);
        }

        SDL_SetRenderDrawColor(gRenderer, 30, 30, 30, 255);
        SDL_RenderClear(gRenderer);

        render();

        SDL_RenderPresent(gRenderer);

        SDL_Delay(10);
    }

    closeSDL();
    return 0;
}


bool initSDL() 
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) 
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    gWindow = SDL_CreateWindow("SDL Drawing Tasks", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (gWindow == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }

    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (gRenderer == NULL) {
        printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(gWindow);
        SDL_Quit();
        return false;
    }

    SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

    printf("SDL Initialized Successfully.\n");
    printf("Controls:\n");
    printf("  Left Click: Draw/Select/Place based on mode\n");
    printf("  Key 1: Draw Lines Mode\n");
    printf("  Key 2: Draw Rectangles Mode\n");
    printf("  Key 3: Draw Circles Mode\n");
    printf("  Key 4: Select/Drag Circles Mode\n");
    printf("  Ctrl+C: Clear Drawings\n");
    printf("  Esc: Quit\n");

    return true;
}

void closeSDL() 
{
    if (gRenderer) 
    {
        SDL_DestroyRenderer(gRenderer);
        gRenderer = NULL;
    }

    if (gWindow) 
    {
        SDL_DestroyWindow(gWindow);
        gWindow = NULL;
    }

    SDL_Quit();
}

void handleEvents(SDL_Event* e, bool* quit) 
{
    if (e->type == SDL_QUIT) 
    {
        *quit = true;
    }
    else if (e->type == SDL_MOUSEMOTION) 
    {
        int mouseX = e->motion.x;
        int mouseY = e->motion.y;

        if (gDraggingCircleIndex != -1) 
        {
            gCircles[gDraggingCircleIndex].x = mouseX - gDragOffsetX;
            gCircles[gDraggingCircleIndex].y = mouseY - gDragOffsetY;
        }
    }
    else if (e->type == SDL_MOUSEBUTTONDOWN) 
    {
        int mouseX = e->button.x;
        int mouseY = e->button.y;

        if (mouseY >= SCREEN_HEIGHT - PALETTE_HEIGHT) 
        {
            int boxIndex = mouseX / PALETTE_BOX_WIDTH;
            if (boxIndex >= 0 && boxIndex < gPaletteColorCount) 
            {
                gCurrentColor = gPaletteColors[boxIndex];
                printf("Color selected: R=%d G=%d B=%d\n", gCurrentColor.r, gCurrentColor.g, gCurrentColor.b);
            }
            return;
        }

        switch (gCurrentMode) 
        {
            case MODE_DRAW_LINE:
                if (!gIsDrawingLine) 
                {
                    gLineStartX = mouseX;
                    gLineStartY = mouseY;
                    gIsDrawingLine = true;
                    printf("Line start: (%d, %d)\n", mouseX, mouseY);
                } else {
                    if (gLineCount < MAX_LINES) {
                        gLines[gLineCount].x1 = gLineStartX;
                        gLines[gLineCount].y1 = gLineStartY;
                        gLines[gLineCount].x2 = mouseX;
                        gLines[gLineCount].y2 = mouseY;
                        gLines[gLineCount].color = gCurrentColor;
                        gLineCount++;
                        printf("Line end: (%d, %d). Count: %d\n", mouseX, mouseY, gLineCount);
                    } else {
                        printf("Max lines reached!\n");
                    }
                    gIsDrawingLine = false;
                }
                break;

            case MODE_DRAW_RECT:
                 if (!gIsDrawingRect) 
                 {
                    gRectStartX = mouseX;
                    gRectStartY = mouseY;
                    gIsDrawingRect = true;
                     printf("Rect start: (%d, %d)\n", mouseX, mouseY);
                } 
                else 
                {
                    if (gRectangleCount < MAX_LINES) {
                        int x = (mouseX < gRectStartX) ? mouseX : gRectStartX;
                        int y = (mouseY < gRectStartY) ? mouseY : gRectStartY;
                        int w = abs(mouseX - gRectStartX);
                        int h = abs(mouseY - gRectStartY);

                        gRectangles[gRectangleCount].x = x;
                        gRectangles[gRectangleCount].y = y;
                        gRectangles[gRectangleCount].w = w;
                        gRectangles[gRectangleCount].h = h;
                        gRectangles[gRectangleCount].color = gCurrentColor;
                        gRectangleCount++;
                        printf("Rect end: (%d, %d). Count: %d\n", mouseX, mouseY, gRectangleCount);
                    } 
                    else 
                    {
                         printf("Max rectangles reached!\n");
                    }
                    gIsDrawingRect = false;
                }
                break;

             case MODE_DRAW_CIRCLE:
                if (gCircleCount < MAX_CIRCLES) 
                {
                    set_circle_data(&gCircles[gCircleCount], mouseX, mouseY, 30.0);
                    set_circle_color(&gCircles[gCircleCount], gCurrentColor);
                    gCircleCount++;
                    printf("Circle added at (%d, %d). Count: %d\n", mouseX, mouseY, gCircleCount);
                } 
                else 
                {
                    printf("Max circles reached!\n");
                }
                break;

             case MODE_SELECT_DRAG:
                 gDraggingCircleIndex = -1;
                 for (int i = gCircleCount - 1; i >= 0; --i) 
                 {
                    double dx = mouseX - gCircles[i].x;
                    double dy = mouseY - gCircles[i].y;
                    double distSq = dx * dx + dy * dy;
                    if (!isnan(gCircles[i].radius) && gCircles[i].radius > 0 &&
                        distSq < gCircles[i].radius * gCircles[i].radius)
                    {
                        gDraggingCircleIndex = i;
                        gDragOffsetX = mouseX - gCircles[i].x;
                        gDragOffsetY = mouseY - gCircles[i].y;
                        printf("Dragging circle %d\n", i);
                        break;
                    }
                 }
                break;
        }
    }
    else if (e->type == SDL_MOUSEBUTTONUP) 
    {
         if (gDraggingCircleIndex != -1) 
         {
             printf("Stopped dragging circle %d\n", gDraggingCircleIndex);
             gDraggingCircleIndex = -1;
         }
    }
    else if (e->type == SDL_KEYDOWN) 
    {
        switch (e->key.keysym.sym) 
        {
            case SDLK_1:
                gCurrentMode = MODE_DRAW_LINE;
                gIsDrawingLine = false;
                gIsDrawingRect = false;
                printf("Mode changed to: Draw Line\n");
                break;
            case SDLK_2:
                gCurrentMode = MODE_DRAW_RECT;
                 gIsDrawingLine = false;
                 gIsDrawingRect = false;
                printf("Mode changed to: Draw Rectangle\n");
                break;
            case SDLK_3:
                 gCurrentMode = MODE_DRAW_CIRCLE;
                 gIsDrawingLine = false;
                 gIsDrawingRect = false;
                printf("Mode changed to: Draw Circle\n");
                break;
             case SDLK_4:
                 gCurrentMode = MODE_SELECT_DRAG;
                 gIsDrawingLine = false;
                 gIsDrawingRect = false;
                printf("Mode changed to: Select/Drag Circle\n");
                break;
             case SDLK_c:
                 if (SDL_GetModState() & KMOD_CTRL) 
                 {
                    gLineCount = 0;
                    gRectangleCount = 0;
                    gCircleCount = 0;
                    gIsDrawingLine = false;
                    gIsDrawingRect = false;
                    gDraggingCircleIndex = -1;
                    printf("Cleared all drawings.\n");
                 }
                 break;
            case SDLK_ESCAPE:
                *quit = true;
                break;
        }
    }
}


void render() 
{    
    drawLines();
    drawRectangles();
    drawCircles();

    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    if (gIsDrawingLine) 
    {
        SDL_SetRenderDrawColor(gRenderer, gCurrentColor.r, gCurrentColor.g, gCurrentColor.b, 150);
        SDL_RenderDrawLine(gRenderer, gLineStartX, gLineStartY, mouseX, mouseY);
    }
    if (gIsDrawingRect) 
    {
         SDL_SetRenderDrawColor(gRenderer, gCurrentColor.r, gCurrentColor.g, gCurrentColor.b, 150);
         int x = (mouseX < gRectStartX) ? mouseX : gRectStartX;
         int y = (mouseY < gRectStartY) ? mouseY : gRectStartY;
         int w = abs(mouseX - gRectStartX);
         int h = abs(mouseY - gRectStartY);
         SDL_Rect rect = { x, y, w, h };
         SDL_RenderDrawRect(gRenderer, &rect);
    }

    drawPalette();
}

void drawPalette() {
    int startY = SCREEN_HEIGHT - PALETTE_HEIGHT;
    for (int i = 0; i < gPaletteColorCount; ++i) {
        SDL_Rect colorBox = { i * PALETTE_BOX_WIDTH, startY, PALETTE_BOX_WIDTH, PALETTE_HEIGHT };
        Color c = gPaletteColors[i];
        SDL_SetRenderDrawColor(gRenderer, c.r, c.g, c.b, c.a);
        SDL_RenderFillRect(gRenderer, &colorBox);

        if (c.r == gCurrentColor.r && c.g == gCurrentColor.g && c.b == gCurrentColor.b && c.a == gCurrentColor.a) 
        {
            SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(gRenderer, &colorBox);
            
            SDL_Rect borderInner = { colorBox.x + 1, colorBox.y + 1, colorBox.w - 2, colorBox.h - 2 };
            SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(gRenderer, &borderInner);

        } 
        else 
        {
            SDL_SetRenderDrawColor(gRenderer, 80, 80, 80, 255);
            SDL_RenderDrawRect(gRenderer, &colorBox);
        }
    }
}

void drawLines() 
{
    for (int i = 0; i < gLineCount; ++i) 
    {
        Color c = gLines[i].color;
        SDL_SetRenderDrawColor(gRenderer, c.r, c.g, c.b, c.a);
        SDL_RenderDrawLine(gRenderer, gLines[i].x1, gLines[i].y1, gLines[i].x2, gLines[i].y2);
    }
}

void drawRectangles() 
{
     for (int i = 0; i < gRectangleCount; ++i) 
     {
        Color c = gRectangles[i].color;
        SDL_SetRenderDrawColor(gRenderer, c.r, c.g, c.b, c.a);
        SDL_Rect rect = { gRectangles[i].x, gRectangles[i].y, gRectangles[i].w, gRectangles[i].h };
         SDL_RenderFillRect(gRenderer, &rect);
    }
}

void drawCircleApprox(SDL_Renderer* renderer, int centerX, int centerY, int radius, Color color, int num_segments) {
    if (radius <= 0 || num_segments < 3) return;

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    double angle_step = 2.0 * M_PI / num_segments;
    double angle = 0.0;

    int start_x = centerX + (int)(radius * cos(angle));
    int start_y = centerY + (int)(radius * sin(angle));
    int prev_x = start_x;
    int prev_y = start_y;

    for (int i = 1; i <= num_segments; ++i) 
    {
        angle = i * angle_step + 1e-9;
        int next_x = centerX + (int)(radius * cos(angle));
        int next_y = centerY + (int)(radius * sin(angle));
        SDL_RenderDrawLine(renderer, prev_x, prev_y, next_x, next_y);
        prev_x = next_x;
        prev_y = next_y;
    }
}

void drawCircleApproxAngleStep(SDL_Renderer* renderer, int centerX, int centerY, int radius, Color color, double angleStepRad) 
{
    if (radius <= 0 || angleStepRad <= 0 || angleStepRad >= M_PI) return;

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    double angle = 0.0;
    int prev_x = centerX + radius;
    int prev_y = centerY;
    int first_x = prev_x;
    int first_y = prev_y;

    while (angle < 2.0 * M_PI) 
    {
        angle += angleStepRad;
        double currentAngle = fmin(angle, 2.0 * M_PI);
        int next_x = centerX + (int)(radius * cos(currentAngle));
        int next_y = centerY + (int)(radius * sin(currentAngle));
        SDL_RenderDrawLine(renderer, prev_x, prev_y, next_x, next_y);
        prev_x = next_x;
        prev_y = next_y;
    }
    if (prev_x != first_x || prev_y != first_y) 
    {
        SDL_RenderDrawLine(renderer, prev_x, prev_y, first_x, first_y);
    }
}

void drawCircleApproxMaxLen(SDL_Renderer* renderer, int centerX, int centerY, int radius, Color color, double maxSegmentLen) {
    if (radius <= 0 || maxSegmentLen <= 0) return;

    double angleStepRad;
    if (maxSegmentLen >= 2.0 * radius) 
    {
        angleStepRad = M_PI / 1.5;
    } 
    else 
    {
        double ratio = fmax(-1.0, fmin(1.0, maxSegmentLen / (2.0 * radius)));
        angleStepRad = 2.0 * asin(ratio);
    }

    int num_segments = (angleStepRad > 1e-6) ? (int)ceil(2.0 * M_PI / angleStepRad) : 360;
    if (num_segments < 4) num_segments = 4;

    drawCircleApprox(renderer, centerX, centerY, radius, color, num_segments);
}


void drawCircles() {
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    bool highlightDrawn[MAX_CIRCLES] = {0};

    if (gCurrentMode == MODE_SELECT_DRAG) 
    {
        for (int i = 0; i < gCircleCount; ++i) 
        {
            if (i == gDraggingCircleIndex) continue;
            if (isnan(gCircles[i].radius) || gCircles[i].radius <= 0) continue;

            double dx = mouseX - gCircles[i].x;
            double dy = mouseY - gCircles[i].y;
            double distSq = dx * dx + dy * dy;

            if (distSq < gCircles[i].radius * gCircles[i].radius) {
                drawCircleHighlight(&gCircles[i]);
                highlightDrawn[i] = true;
            }
        }
    }


    for (int i = 0; i < gCircleCount; ++i) 
    {
        if (isnan(gCircles[i].radius) || gCircles[i].radius <= 0) continue;

        int num_segments = 36;
        if (gCircles[i].radius > 10) num_segments = (int)(gCircles[i].radius * 1.5);
        if (num_segments < 12) num_segments = 12;
        if (num_segments > 100) num_segments = 100;

        Color c = gCircles[i].color;

        if (i == gDraggingCircleIndex) {
            c.r = (Uint8)fmin(255.0, c.r * 1.2 + 30);
            c.g = (Uint8)fmin(255.0, c.g * 1.2 + 30);
            c.b = (Uint8)fmin(255.0, c.b * 1.2 + 30);
            c.a = 200;
        }

        drawCircleApprox(gRenderer, (int)gCircles[i].x, (int)gCircles[i].y, (int)gCircles[i].radius, c, num_segments);
    }
}

void drawCircleHighlight(const Circle* circle) 
{
     if (isnan(circle->radius) || circle->radius <= 0) return;

    int cx = (int)circle->x;
    int cy = (int)circle->y;
    int markerSize = (int)(circle->radius * 0.3);
    if (markerSize < 3) markerSize = 3;
    if (markerSize > circle->radius * 0.8) markerSize = (int)(circle->radius * 0.8);

    Color highlightColor = {255, 255, 255, 255};
    if (circle->color.r + circle->color.g + circle->color.b > 3 * 128) 
    {
        highlightColor = (Color){0, 0, 0, 255};
    }

    SDL_SetRenderDrawColor(gRenderer, highlightColor.r, highlightColor.g, highlightColor.b, 200);

    SDL_RenderDrawLine(gRenderer, cx - markerSize, cy, cx + markerSize, cy);
    SDL_RenderDrawLine(gRenderer, cx, cy - markerSize, cx, cy + markerSize);

}