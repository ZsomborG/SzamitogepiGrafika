// Temporarily update texture.c to use GLAD before the stb_image switch
#include "texture.h"

#include <SDL2/SDL_image.h> // Still using SDL_image for now
#include <stdio.h>         // For error messages
#include <glad/glad.h>     // Use GLAD

GLuint load_texture(const char* filename) // Use const char*
{
    SDL_Surface* surface;
    GLuint texture_name = 0; // Initialize to 0 (error indicator)

    surface = IMG_Load(filename);
    if (!surface) {
        fprintf(stderr, "[ERROR] IMG_Load: %s\n", IMG_GetError());
        return 0;
    }

    // Check format, convert if necessary (example: ensure RGB)
    // For simplicity, assume format is usable or convert it
    // This example assumes we want GL_RGB
    GLenum texture_format;
    GLint internal_format;
    if (surface->format->BytesPerPixel == 4) {
        // Has alpha
        if (surface->format->Rmask == 0x000000ff) { // RGBA
             texture_format = GL_RGBA;
             internal_format = GL_RGBA;
         } else { // BGRA? SDL might handle endianness. Check masks needed.
             texture_format = GL_BGRA; // Common on some systems
             internal_format = GL_RGBA; // Request RGBA internal storage
             printf("[WARN] Texture '%s' has BGRA format, check if handled correctly by GL.\n", filename);
         }
    } else if (surface->format->BytesPerPixel == 3) {
        // No alpha
        if (surface->format->Rmask == 0x000000ff) { // RGB
             texture_format = GL_RGB;
             internal_format = GL_RGB;
        } else { // BGR?
            texture_format = GL_BGR; // Common on some systems
            internal_format = GL_RGB; // Request RGB internal storage
            printf("[WARN] Texture '%s' has BGR format, check if handled correctly by GL.\n", filename);
        }
    } else {
        fprintf(stderr, "[ERROR] Texture '%s': Unrecognized image format (BPP=%d).\n", filename, surface->format->BytesPerPixel);
        SDL_FreeSurface(surface);
        return 0;
    }


    glGenTextures(1, &texture_name);
    glBindTexture(GL_TEXTURE_2D, texture_name);

    // Upload texture data
    glTexImage2D(GL_TEXTURE_2D, 0,           // level
                 internal_format,            // internal format (how GL stores it)
                 surface->w, surface->h, 0,  // width, height, border
                 texture_format,             // format of source data
                 GL_UNSIGNED_BYTE,           // type of source data
                 surface->pixels);

    // Set texture parameters - Mipmapping is generally preferred
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Generate mipmaps AFTER uploading base level
    glGenerateMipmap(GL_TEXTURE_2D);

    // Set wrap modes (optional, default is often GL_REPEAT)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Use REPEAT instead of CLAMP
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    SDL_FreeSurface(surface); // Free the SDL surface memory

    glBindTexture(GL_TEXTURE_2D, 0); // Unbind

    printf("[INFO] Texture loaded via SDL_image: '%s' (ID: %u)\n", filename, texture_name);

    return texture_name;
}