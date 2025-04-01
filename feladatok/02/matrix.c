#include "matrix.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

void init_zero_matrix(float matrix[3][3])
{
    int i;
    int j;

    for (i = 0; i < 3; ++i) 
    {
        for (j = 0; j < 3; ++j) 
        {
            matrix[i][j] = 0.0f;
        }
    }
}

void init_identity_matrix(float matrix[3][3])
{
    int i;
    int j;

    for (i = 0; i < 3; ++i) 
    {
        for (j = 0; j < 3; ++j) 
        {
            if (i == j) {
                matrix[i][j] = 1.0f;
            } else {
                matrix[i][j] = 0.0f;
            }
        }
    }
}

void copy_matrix(const float source[3][3], float destination[3][3])
{
    int i;
    int j;
    
    for (i = 0; i < 3; ++i) 
    {
        for (j = 0; j < 3; ++j) 
        {
            destination[i][j] = source[i][j];
        }
    }
}


void print_matrix(const float matrix[3][3])
{
    int i;
    int j;

    printf("[\n");
    for (i = 0; i < 3; ++i) 
    {
        printf("  ");
        for (j = 0; j < 3; ++j) 
        {
            printf("%8.4f ", matrix[i][j]);
        }
        printf("\n");
    }
     printf("]\n");
}

void add_matrices(const float a[3][3], const float b[3][3], float c[3][3])
{
    int i;
    int j;

    for (i = 0; i < 3; ++i) 
    {
        for (j = 0; j < 3; ++j) 
        {
            c[i][j] = a[i][j] + b[i][j];
        }
    }
}

void scalar_multiply(float matrix[3][3], float scalar)
{
    int i;
    int j;

    for (i = 0; i < 3; ++i) 
    {
        for (j = 0; j < 3; ++j) 
        {
            matrix[i][j] *= scalar;
        }
    }
}

void multiply_matrices(const float a[3][3], const float b[3][3], float c[3][3])
{
    int i;
    int j;
    int k;
    float temp_result[3][3];

    for (i = 0; i < 3; ++i) 
    {
        for (j = 0; j < 3; ++j) 
        {
            temp_result[i][j] = 0.0f;
            for (k = 0; k < 3; ++k) 
            {
                temp_result[i][j] += a[i][k] * b[k][j];
            }
        }
    }

    copy_matrix(temp_result, c);
}


void transform_point(const float matrix[3][3], const float point[3], float result[3])
{
    int i;
    int k;
    float temp_result[3];

    for (i = 0; i < 3; ++i) 
    {
        temp_result[i] = 0.0f;
        for (k = 0; k < 3; ++k) 
        {
            temp_result[i] += matrix[i][k] * point[k];
        }
    }

    for (i = 0; i < 3; ++i) 
    {
        result[i] = temp_result[i];
    }
}

void scale(float matrix[3][3], float sx, float sy)
{
    float scale_matrix[3][3];
    float temp_matrix[3][3];

    init_identity_matrix(scale_matrix);
    scale_matrix[0][0] = sx;
    scale_matrix[1][1] = sy;

    copy_matrix(matrix, temp_matrix);

    multiply_matrices(scale_matrix, temp_matrix, matrix);
}

void shift(float matrix[3][3], float dx, float dy)
{
    float shift_matrix[3][3];
    float temp_matrix[3][3];

    init_identity_matrix(shift_matrix);
    shift_matrix[0][2] = dx;
    shift_matrix[1][2] = dy;

    copy_matrix(matrix, temp_matrix);
    multiply_matrices(shift_matrix, temp_matrix, matrix);
}

void rotate(float matrix[3][3], float angle)
{
    float rotate_matrix[3][3];
    float temp_matrix[3][3];
    float cos_angle = cosf(angle);
    float sin_angle = sinf(angle);

    init_identity_matrix(rotate_matrix);
    rotate_matrix[0][0] = cos_angle;
    rotate_matrix[0][1] = -sin_angle;
    rotate_matrix[1][0] = sin_angle;
    rotate_matrix[1][1] = cos_angle;

    copy_matrix(matrix, temp_matrix);
    multiply_matrices(rotate_matrix, temp_matrix, matrix);
}


void init_stack(MatrixStack* stack)
{
    stack->top = -1;
}

int push_matrix(MatrixStack* stack, const float matrix[3][3])
{
    if (stack->top >= MAX_STACK_DEPTH - 1) 
    {
        fprintf(stderr, "Error: Matrix stack overflow!\n");
        return -1;
    }
    stack->top++;
    copy_matrix(matrix, stack->matrices[stack->top]);
    return 0;
}

int pop_matrix(MatrixStack* stack, float matrix[3][3])
{
    if (stack->top < 0) 
    {
        fprintf(stderr, "Error: Matrix stack underflow!\n");
        return -1;
    }
    copy_matrix(stack->matrices[stack->top], matrix);
    stack->top--;
    return 0;
}