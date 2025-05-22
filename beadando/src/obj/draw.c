#include <obj/draw.h>
#include <obj/model.h> // Need model struct definition

#include <glad/glad.h> // Need GL functions

#include <stdio.h> // For errors or info

void draw_model(const Model* model)
{
    if (!model || model->vao_id == 0 || model->index_count == 0) {
        printf("DEBUG: draw_model - SKIPPING (Invalid model/vao/indices)\n");
        return;
    }
    // printf("DEBUG: draw_model - Binding VAO %u\n", model->vao_id);
    glBindVertexArray(model->vao_id);

    // printf("DEBUG: draw_model - Calling glDrawElements (Count: %d)\n", model->index_count);
    glDrawElements(
            GL_TRIANGLES,
            model->index_count,
            GL_UNSIGNED_INT,
            NULL
    );

    // printf("DEBUG: draw_model - Unbinding VAO\n");
    glBindVertexArray(0);
    // printf("DEBUG: draw_model - END\n");
}

// This function might not be needed if draw_model handles everything
void draw_triangles(const Model* model) {
    (void)model;
    // If you need a separate way to draw raw triangles without VAO/VBO setup
    // (e.g., immediate mode style for debugging), implement here.
    // Otherwise, this can likely be removed or left empty.
}