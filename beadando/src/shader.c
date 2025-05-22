#include "shader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h> // For strlen, strcmp

// Helper function to read shader source code from a file
// Returns a dynamically allocated string containing the file content,
// or NULL if the file couldn't be read. Caller must free the memory.
char* read_shader_source(const char* path) {
    FILE* file = fopen(path, "rb"); // Read in binary mode to avoid Windows CRLF issues
    if (!file) {
        fprintf(stderr, "ERROR: Could not open shader file: %s\n", path);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (length <= 0) { // Handle empty file or error
         fprintf(stderr, "ERROR: Shader file is empty or error reading size: %s\n", path);
         fclose(file);
         return NULL;
    }

    char* buffer = (char*)malloc(length + 1); // +1 for null terminator
    if (!buffer) {
        fprintf(stderr, "ERROR: Could not allocate memory for shader source: %s\n", path);
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(buffer, 1, length, file);
    fclose(file);

    if (bytes_read != length) {
         fprintf(stderr, "ERROR: Could not read entire shader file: %s\n", path);
         free(buffer);
         return NULL;
    }

    buffer[length] = '\0'; // Null-terminate the string

    return buffer;
}

// Helper function to compile a shader and check for errors
// Returns the shader ID, or 0 if compilation failed.
GLuint compile_shader(const char* source, GLenum type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[1024]; // Buffer for error message
        glGetShaderInfoLog(shader, sizeof(infoLog), NULL, infoLog);
        fprintf(stderr, "ERROR: SHADER COMPILATION FAILED (%s):\n%s\n------------------------------------------------------\n",
                (type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT"), infoLog);
        glDeleteShader(shader); // Clean up
        return 0; // Return 0 on failure
    }
    return shader;
}



// Main function to load shaders
GLuint load_shaders(const char* vertex_path, const char* fragment_path) {
    char* vertex_source = read_shader_source(vertex_path);
    if (!vertex_source) return 0;

    char* fragment_source = read_shader_source(fragment_path);
    if (!fragment_source) {
        free(vertex_source); // Clean up vertex source if fragment fails
        return 0;
    }

    GLuint vertex_shader = compile_shader(vertex_source, GL_VERTEX_SHADER);
    free(vertex_source); // Free source after compilation attempt
    if (vertex_shader == 0) {
        if(fragment_source) free(fragment_source); // Ensure fragment source is freed if allocated
        return 0; // Vertex shader failed
    }

    GLuint fragment_shader = compile_shader(fragment_source, GL_FRAGMENT_SHADER);
    free(fragment_source); // Free source after compilation attempt
    if (fragment_shader == 0) {
        glDeleteShader(vertex_shader); // Clean up successful vertex shader
        return 0; // Fragment shader failed
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[1024];
        glGetProgramInfoLog(program, sizeof(infoLog), NULL, infoLog);
        fprintf(stderr, "ERROR: SHADER PROGRAM LINKING FAILED:\n%s\n------------------------------------------------------\n", infoLog);
        glDeleteShader(vertex_shader);   // Don't leak shaders
        glDeleteShader(fragment_shader);
        glDeleteProgram(program);      // Don't leak program
        return 0; // Linking failed
    }

    // Detach and delete shaders after successful link
    glDetachShader(program, vertex_shader);
    glDetachShader(program, fragment_shader);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    printf("[INFO] Shaders loaded and linked successfully (Program ID: %u)\n", program);
    return program;
}