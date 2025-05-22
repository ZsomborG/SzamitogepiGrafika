#include "utils.h"
// #include <cglm/cglm.h> // Not directly needed by this function
// #include <math.h>      // Not directly needed by this function
#include <glad/glad.h> // For GL types and glGetError()
#include <stdio.h>     // For fprintf, stderr

void _check_gl_error(const char *file, int line, const char* operation_name) { // Changed signature
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        fprintf(stderr, "OpenGL Error @ %s:%d (after %s): %u (0x%x) - ", file, line, operation_name, err, err); // Use file and line
        switch (err) {
            case GL_INVALID_ENUM: fprintf(stderr, "GL_INVALID_ENUM\n"); break;
            case GL_INVALID_VALUE: fprintf(stderr, "GL_INVALID_VALUE\n"); break;
            case GL_INVALID_OPERATION: fprintf(stderr, "GL_INVALID_OPERATION\n"); break;
            case GL_STACK_OVERFLOW: fprintf(stderr, "GL_STACK_OVERFLOW\n"); break;
            case GL_STACK_UNDERFLOW: fprintf(stderr, "GL_STACK_UNDERFLOW\n"); break;
            case GL_OUT_OF_MEMORY: fprintf(stderr, "GL_OUT_OF_MEMORY\n"); break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: fprintf(stderr, "GL_INVALID_FRAMEBUFFER_OPERATION\n"); break;
            default: fprintf(stderr, "Unknown error\n"); break;
        }
    }
}