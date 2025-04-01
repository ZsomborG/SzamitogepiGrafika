#ifndef MATRIX_H
#define MATRIX_H

#include <stdio.h>
#include <math.h>

#define MAX_STACK_DEPTH 32

void init_zero_matrix(float matrix[3][3]);

void init_identity_matrix(float matrix[3][3]);

void copy_matrix(const float source[3][3], float destination[3][3]);

void print_matrix(const float matrix[3][3]);

void add_matrices(const float a[3][3], const float b[3][3], float c[3][3]);

void scalar_multiply(float matrix[3][3], float scalar);

void multiply_matrices(const float a[3][3], const float b[3][3], float c[3][3]);

void transform_point(const float matrix[3][3], const float point[3], float result[3]);

void scale(float matrix[3][3], float sx, float sy);

void shift(float matrix[3][3], float dx, float dy);

void rotate(float matrix[3][3], float angle);

typedef struct 
{
    float matrices[MAX_STACK_DEPTH][3][3];
    int top;
} MatrixStack;

void init_stack(MatrixStack* stack);

int push_matrix(MatrixStack* stack, const float matrix[3][3]);

int pop_matrix(MatrixStack* stack, float matrix[3][3]);

#endif