#include <obj/model.h>
#include <stdio.h>
#include <stdlib.h> // For malloc, free
#include <string.h> // For memcpy, NULL

// Include GLAD for OpenGL functions
#include <glad/glad.h>
#include "model.h"


void init_model(Model* model)
{
    model->n_vertices = 0;
    model->n_texture_vertices = 0;
    model->n_normals = 0;
    model->n_triangles = 0;
    model->vertices = NULL;
    model->texture_vertices = NULL;
    model->normals = NULL;
    model->triangles = NULL;
    model->vao_id = 0;
    model->vbo_id = 0;
    model->ibo_id = 0;
    model->index_count = 0;
}

int allocate_model(Model* model)
{
    printf("DEBUG: allocate_model - START (Requesting V=%d, VT=%d, VN=%d, F=%d)\n",
        model->n_vertices, model->n_texture_vertices, model->n_normals, model->n_triangles);
    if (model->n_vertices > 0) {
        model->vertices = (Vertex*)malloc(model->n_vertices * sizeof(Vertex));
        if (model->vertices == NULL) {
            fprintf(stderr, "ERROR: Failed to allocate memory for vertices.\n");
            return FALSE;
        }
    }
    if (model->n_texture_vertices > 0) {
        model->texture_vertices = (TextureVertex*)malloc(model->n_texture_vertices * sizeof(TextureVertex));
         if (model->texture_vertices == NULL) {
            fprintf(stderr, "ERROR: Failed to allocate memory for texture vertices.\n");
            // Free previously allocated memory before returning
            free(model->vertices); model->vertices = NULL;
            return FALSE;
        }
    }
    if (model->n_normals > 0) {
        model->normals = (Vertex*)malloc(model->n_normals * sizeof(Vertex));
         if (model->normals == NULL) {
            fprintf(stderr, "ERROR: Failed to allocate memory for normals.\n");
            free(model->vertices); model->vertices = NULL;
            free(model->texture_vertices); model->texture_vertices = NULL;
            return FALSE;
        }
    }
    if (model->n_triangles > 0) {
        model->triangles = (Triangle*)malloc(model->n_triangles * sizeof(Triangle));
         if (model->triangles == NULL) {
            fprintf(stderr, "ERROR: Failed to allocate memory for triangles.\n");
            free(model->vertices); model->vertices = NULL;
            free(model->texture_vertices); model->texture_vertices = NULL;
            free(model->normals); model->normals = NULL;
            return FALSE;
        }
    }
    printf("[INFO] Model memory allocated: V=%d, VT=%d, VN=%d, F=%d\n",
        model->n_vertices, model->n_texture_vertices, model->n_normals, model->n_triangles);
    return TRUE;
}

void free_model(Model* model)
{
    if (model == NULL) return;

    free(model->vertices);
    free(model->texture_vertices);
    free(model->normals);
    free(model->triangles);

    // Reset model state after freeing
    init_model(model);
    // Note: This does NOT free the GPU buffers (VAO/VBO/IBO).
    // Call destroy_model_buffers separately.
}

// --- VBO/VAO/IBO Setup ---

// Define a structure for the interleaved vertex data we will send to the GPU
// Using floats as preferred by OpenGL
typedef struct VertexData {
    float position[3];
    float normal[3];
    float tex_coord[2];
} VertexData;


void setup_model_buffers(Model* model) {
    printf("DEBUG: setup_model_buffers - START\n");
    if (!model || !model->triangles || model->n_triangles == 0 || !model->vertices) {
        fprintf(stderr, "ERROR: Cannot setup model buffers - model data missing or empty.\n");
        return;
    }

    // --- 1. Prepare data for buffers ---
    // We need to potentially combine data from the separate arrays loaded from the OBJ
    // into a single interleaved VBO and create an index buffer (IBO/EBO).

    // Calculate the number of unique face points (indices) needed.
    // This is simply the number of corners in all triangles.
    GLsizei num_indices = model->n_triangles * 3;
    model->index_count = num_indices; // Store for drawing

    // Allocate memory for vertex data and index data
    VertexData* vertex_buffer_data = (VertexData*)malloc(num_indices * sizeof(VertexData));
    GLuint* index_buffer_data = (GLuint*)malloc(num_indices * sizeof(GLuint));

    if (!vertex_buffer_data || !index_buffer_data) {
        fprintf(stderr, "ERROR: Failed to allocate memory for buffer setup.\n");
        free(vertex_buffer_data); // free if allocated
        free(index_buffer_data);  // free if allocated
        return;
    }

    // Populate the buffers by iterating through the triangles
    for (int i = 0; i < model->n_triangles; ++i) {
        for (int j = 0; j < 3; ++j) { // For each point of the triangle
            int current_index = i * 3 + j;
            FacePoint fp = model->triangles[i].points[j];

            // --- Index Validation ---
            if (fp.vertex_index <= 0 || fp.vertex_index > model->n_vertices) {
                fprintf(stderr, "ERROR: setup_model_buffers - Invalid vertex index %d in face %d point %d.\n", fp.vertex_index, i, j);
                // Handle error: free allocated buffers and return?
                free(vertex_buffer_data);
                free(index_buffer_data);
                return;
            }
            if (model->texture_vertices && (fp.texture_index <= 0 || fp.texture_index > model->n_texture_vertices)) {
                fprintf(stderr, "ERROR: setup_model_buffers - Invalid texture index %d in face %d point %d.\n", fp.texture_index, i, j);
                free(vertex_buffer_data); free(index_buffer_data); return;
            }
            if (model->normals && (fp.normal_index <= 0 || fp.normal_index > model->n_normals)) {
                fprintf(stderr, "ERROR: setup_model_buffers - Invalid normal index %d in face %d point %d.\n", fp.normal_index, i, j);
                free(vertex_buffer_data); free(index_buffer_data); return;
            }
            // --- End Index Validation ---

            // Get data from the model arrays using the indices from the FacePoint
            // Remember OBJ indices are 1-based, C arrays are 0-based!
            Vertex pos = model->vertices[fp.vertex_index - 1];
            TextureVertex tex = {0.0, 0.0}; // Default tex coord if missing
            if (model->texture_vertices && fp.texture_index > 0) {
                tex = model->texture_vertices[fp.texture_index - 1];
            }
            Vertex norm = {0.0, 0.0, 1.0}; // Default normal if missing
             if (model->normals && fp.normal_index > 0) {
                 norm = model->normals[fp.normal_index - 1];
             }

            // Copy data into the vertex buffer (converting double to float)
            vertex_buffer_data[current_index].position[0] = (float)pos.x;
            vertex_buffer_data[current_index].position[1] = (float)pos.y;
            vertex_buffer_data[current_index].position[2] = (float)pos.z;

            vertex_buffer_data[current_index].normal[0] = (float)norm.x;
            vertex_buffer_data[current_index].normal[1] = (float)norm.y;
            vertex_buffer_data[current_index].normal[2] = (float)norm.z;

            vertex_buffer_data[current_index].tex_coord[0] = (float)tex.u;
            vertex_buffer_data[current_index].tex_coord[1] = (float)tex.v;

            // Set the index buffer data - just a sequence 0, 1, 2, 3...
            // because we are duplicating vertices as needed for unique combinations.
            index_buffer_data[current_index] = current_index;
        }
    }

    // --- 2. Create and Bind VAO ---
    glGenVertexArrays(1, &model->vao_id);
    glBindVertexArray(model->vao_id);

    // --- 3. Create, Bind, and Fill VBO ---
    glGenBuffers(1, &model->vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, model->vbo_id);
    glBufferData(GL_ARRAY_BUFFER, num_indices * sizeof(VertexData), vertex_buffer_data, GL_STATIC_DRAW);

    // --- 4. Create, Bind, and Fill IBO ---
    glGenBuffers(1, &model->ibo_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->ibo_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices * sizeof(GLuint), index_buffer_data, GL_STATIC_DRAW);

    // --- 5. Configure Vertex Attributes ---
    // Tell OpenGL how the data is laid out in the VBO

    // Attribute 0: Position (matches layout (location = 0) in vertex shader)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, // Attribute index 0
        3,                  // Number of components (x, y, z)
        GL_FLOAT,           // Type of components
        GL_FALSE,           // Normalized?
        sizeof(VertexData), // Stride (bytes between consecutive vertices)
        (void*)offsetof(VertexData, position) // Offset of the first component
    );

    // Attribute 1: Normal (assuming layout (location = 1) in vertex shader - ADJUST SHADER IF NEEDED)
    // NOTE: Our current simple shader doesn't use normals, but we upload them for future lighting.
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, // Attribute index 1
        3,                  // Number of components (nx, ny, nz)
        GL_FLOAT,           // Type
        GL_FALSE,           // Normalized? (Could be GL_TRUE if normals aren't unit length)
        sizeof(VertexData), // Stride
        (void*)offsetof(VertexData, normal) // Offset
    );


    // Attribute 2: Texture Coordinate (matches layout (location = 1) in current vertex shader)
    // RENUMBER LAYOUT LOCATIONS IN SHADER: Pos=0, TexCoord=1 (Normal might be 2 later)
    glEnableVertexAttribArray(2); // Use attribute index 2 for tex coords now
    glVertexAttribPointer(
        2, // Attribute index 2
        2,                  // Number of components (u, v)
        GL_FLOAT,           // Type
        GL_FALSE,           // Normalized?
        sizeof(VertexData), // Stride
        (void*)offsetof(VertexData, tex_coord) // Offset
    );

    // --- 6. Unbind VAO and Buffers (optional but good practice) ---
    glBindVertexArray(0); // Unbind VAO first!
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // Unbind IBO after VAO

    // --- 7. Free temporary CPU buffers ---
    free(vertex_buffer_data);
    free(index_buffer_data);

    printf("[INFO] Model buffers created: VAO=%u, VBO=%u, IBO=%u, Indices=%d\n",
           model->vao_id, model->vbo_id, model->ibo_id, model->index_count);
}

// Delete OpenGL buffers associated with the model
void destroy_model_buffers(Model* model) {
    printf("DEBUG: Destroying model buffers...\n");
    if (!model) return;

    if (model->ibo_id != 0) {
        glDeleteBuffers(1, &model->ibo_id);
        model->ibo_id = 0;
    }
    if (model->vbo_id != 0) {
        glDeleteBuffers(1, &model->vbo_id);
        model->vbo_id = 0;
    }
    if (model->vao_id != 0) {
        glDeleteVertexArrays(1, &model->vao_id);
        model->vao_id = 0;
    }
     model->index_count = 0;
    printf("[INFO] Model buffers destroyed.\n");
}