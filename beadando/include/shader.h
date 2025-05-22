#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h> // For GLuint type

/**
 * @brief Loads, compiles, and links vertex and fragment shaders into a shader program.
 *
 * Reads shader source code from files, compiles them, checks for errors,
 * links them into a program, checks for linking errors, and cleans up.
 *
 * @param vertex_path Path to the vertex shader source file.
 * @param fragment_path Path to the fragment shader source file.
 * @return The ID of the linked shader program, or 0 if an error occurred.
 */
GLuint load_shaders(const char* vertex_path, const char* fragment_path);

#endif // SHADER_H