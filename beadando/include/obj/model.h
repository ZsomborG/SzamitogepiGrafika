#ifndef OBJ_MODEL_H
#define OBJ_MODEL_H

#define TRUE 1
#define FALSE 0

#define INVALID_VERTEX_INDEX 0

#include <glad/glad.h>

/**
 * Three dimensional vertex
 */
typedef struct Vertex
{
    double x;
    double y;
    double z;
} Vertex;

/**
 * Two dimensional texture vertex
 */
typedef struct TextureVertex
{
    double u;
    double v;
} TextureVertex;

/**
 * Point of the face
 */
typedef struct FacePoint
{
    int vertex_index;
    int texture_index;
    int normal_index;
} FacePoint;

/**
 * Triangle as facepoint triplet
 */
typedef struct Triangle
{
    struct FacePoint points[3];
} Triangle;

/**
 * Three dimensional model with texture
 */
typedef struct Model
{
    int n_vertices;
    int n_texture_vertices;
    int n_normals;
    int n_triangles;
    Vertex* vertices;
    TextureVertex* texture_vertices;
    Vertex* normals;
    Triangle* triangles;

    GLuint vao_id;
    GLuint vbo_id;
    GLuint ibo_id;
    GLsizei index_count;
} Model;

/**
 * Types of the considered elements
 */
typedef enum {
    NONE,
    VERTEX,
    TEXTURE_VERTEX,
    NORMAL,
    FACE
} ElementType;

/**
 * Initialize the model structure.
 */
void init_model(Model* model);

/**
 * Allocate model.
 */
int allocate_model(Model* model);

/**
 * Release the allocated memory of the model.
 */
void free_model(Model* model);

/**
 * Creates and configures the VAO and VBOs for the loaded model data.
 * Must be called after load_model and before drawing.
 * Requires an active OpenGL context.
 */
 void setup_model_buffers(Model* model);

 /**
  * Deletes the VAO and VBOs associated with the model.
  */
 void destroy_model_buffers(Model* model);

#endif /* OBJ_MODEL_H */
