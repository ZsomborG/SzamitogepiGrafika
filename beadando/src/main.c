#include "app.h"

#include <stdio.h>

/**
 * Main function
 */
int main(int argc, char* argv[])
{
    App app;

    printf("DEBUG: main - Calling init_app...\n");
    init_app(&app, 800, 600);
    printf("DEBUG: main - init_app finished. Checking loop condition (app.is_running=%s)...\n", app.is_running ? "true" : "false");

    while (app.is_running) {
        update_app(&app);
        render_app(&app);
    }
    
    printf("DEBUG: main - Loop finished. Calling destroy_app...\n");
    destroy_app(&app);
    printf("DEBUG: main - Exiting.\n");

    return 0;
}
