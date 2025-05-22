#ifndef UTILS_H
#define UTILS_H

#include <cglm/cglm.h>
#include <glad/glad.h>
#include <stdio.h>
 
 typedef struct Material
 {
     vec3 ambient; // Use vec3 for color representation
     vec3 diffuse;
     vec3 specular;
     float shininess;
 } Material;

void _check_gl_error(const char *file, int line, const char* operation_name);
#define check_gl_error(op_name) _check_gl_error(__FILE__, __LINE__, op_name)

#endif // UTILS_H