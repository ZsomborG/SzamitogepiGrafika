#include "matrix.h"
#include <stdio.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int main(int argc, char* argv[])
{
	printf("--- Original Example: Matrix Addition ---\n");
	float a[3][3] = 
    {
        { 1.0f, -2.0f,  3.0f},
        { 5.0f, -3.0f,  0.0f},
        {-2.0f,  1.0f, -4.0f}
    };
	float b[3][3];
	float c[3][3];

    init_zero_matrix(b);
    b[1][1] =  8.0f;
    b[2][0] = -3.0f;
    b[2][2] =  5.0f;

    printf("Matrix A:\n");
    print_matrix(a);
    printf("Matrix B:\n");
    print_matrix(b);

    add_matrices(a, b, c);
    printf("Result C = A + B:\n");
    print_matrix(c);
    printf("\n");


    printf("--- Example: Identity Matrix ---\n");
    float id[3][3];
    init_identity_matrix(id);
    printf("Identity Matrix ID:\n");
    print_matrix(id);
    printf("\n");

    printf("--- Example: Scalar Multiplication ---\n");
    float scaled_a[3][3];
    copy_matrix(a, scaled_a);
    printf("Matrix A before scaling:\n");
    print_matrix(scaled_a);
    scalar_multiply(scaled_a, 2.5f);
    printf("Matrix A after scaling by 2.5:\n");
    print_matrix(scaled_a);
    printf("\n");

    printf("--- Example: Matrix Multiplication ---\n");
    float d[3][3];
    multiply_matrices(a, b, d);
    printf("Matrix A:\n");
    print_matrix(a);
    printf("Matrix B:\n");
    print_matrix(b);
    printf("Result D = A * B:\n");
    print_matrix(d);
    printf("\n");

    printf("--- Example: Transform Point ---\n");
    float transform_mat[3][3];
    init_identity_matrix(transform_mat);
    shift(transform_mat, 5.0f, 10.0f);
    scale(transform_mat, 2.0f, 0.5f);
    printf("Transformation Matrix T:\n");
    print_matrix(transform_mat);

    float point[3] = { 3.0f, 4.0f, 1.0f };
    float transformed_point[3];
    printf("Original Point P = (%f, %f)\n", point[0], point[1]);

    transform_point(transform_mat, point, transformed_point);
    printf("Transformed Point T*P = (%f, %f)\n", transformed_point[0], transformed_point[1]);
    printf("\n");


    printf("--- Example: Building Transformations ---\n");
    float build_mat[3][3];
    init_identity_matrix(build_mat);
    printf("Initial Matrix (Identity):\n");
    print_matrix(build_mat);

    scale(build_mat, 1.5f, 1.5f);
    printf("After Scaling (1.5, 1.5):\n");
    print_matrix(build_mat);

    shift(build_mat, -2.0f, 3.0f);
    printf("After Shifting (-2, 3):\n");
    print_matrix(build_mat);

    float angle_rad = M_PI / 4.0f;
    rotate(build_mat, angle_rad);
    printf("After Rotating by PI/4 radians (45 deg):\n");
    print_matrix(build_mat);
    printf("\n");

    printf("--- Example: Transformation Stack ---\n");
    MatrixStack stack;
    init_stack(&stack);

    float current_transform[3][3];
    init_identity_matrix(current_transform);
    printf("Initial state (Identity):\n");
    print_matrix(current_transform);

    if (push_matrix(&stack, current_transform) == 0) 
    {
         printf("Pushed initial state onto stack.\n");
    }

    shift(current_transform, 10.0f, 0.0f);
    printf("State after shift(10, 0):\n");
    print_matrix(current_transform);

    if (push_matrix(&stack, current_transform) == 0) 
    {
        printf("Pushed shifted state onto stack.\n");
    }

    scale(current_transform, 2.0f, 2.0f);
     printf("State after scale(2, 2):\n");
    print_matrix(current_transform);

    printf("\nPopping stack...\n");
    if (pop_matrix(&stack, current_transform) == 0) 
    {
        printf("State after pop (should be shifted state):\n");
        print_matrix(current_transform);
    }

    printf("\nPopping stack again...\n");
    if (pop_matrix(&stack, current_transform) == 0) 
    {
        printf("State after pop (should be initial identity state):\n");
        print_matrix(current_transform);
    }

    printf("\nTrying to pop empty stack...\n");
    pop_matrix(&stack, current_transform);

    printf("\n--- All Examples Complete ---\n");

	return 0;
}