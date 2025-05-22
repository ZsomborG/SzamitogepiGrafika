#ifndef TEXTURE_H
#define TEXTURE_H

// Use GLAD's types instead of GL/gl.h
#include <glad/glad.h>

// Pixel definition remains the same if used internally by loader,
// but stb_image handles pixel data directly. Let's remove it for now.
// typedef GLubyte Pixel[3];

/**
 * Load texture from file and returns with the texture name (OpenGL ID).
 * Returns 0 on failure.
 * Takes const char* for filename safety.
 */
GLuint load_texture(const char* filename);

#endif /* TEXTURE_H */