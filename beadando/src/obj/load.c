#include <obj/load.h>
#include <obj/model.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_BUFFER_SIZE 1024

// Forward declarations for functions defined later in this file
void count_elements(Model* model, FILE* file);
int read_elements(Model* model, FILE* file);
ElementType calc_element_type(const char* text);
int read_vertex(Vertex* vertex, const char* text);
int read_texture_vertex(TextureVertex* texture_vertex, const char* text);
int read_normal(Vertex* normal, const char* text);
int read_triangle(Triangle* triangle, const char* text);
int is_numeric(char c);


// Corrected load_model function
int load_model(Model* model, const char* filename)
{
    FILE* file = NULL;
    int allocation_success = FALSE;
    int read_success = FALSE;

    printf("DEBUG: load_model('%s') - START\n", filename);

    init_model(model); // Initialize struct fields to 0/NULL

    file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "ERROR: Cannot open model file: '%s'\n", filename);
        return FALSE;
    }

    printf("DEBUG: load_model - Counting elements...\n");
    count_elements(model, file); // This will read through the file
    printf("DEBUG: load_model - Elements counted (V=%d, VT=%d, VN=%d, F=%d)\n",
           model->n_vertices, model->n_texture_vertices, model->n_normals, model->n_triangles);

    // Check counts before allocating
    if(model->n_vertices == 0 || model->n_triangles == 0) {
        fprintf(stderr, "ERROR: Model file '%s' has no vertices or faces.\n", filename);
        fclose(file);
        return FALSE;
    }

    printf("DEBUG: load_model - Allocating memory...\n");
    allocation_success = allocate_model(model); // Call allocate_model ONLY HERE
    if (!allocation_success) {
        fprintf(stderr, "ERROR: Failed to allocate memory during load_model.\n");
        fclose(file);
        return FALSE;
    }
    printf("DEBUG: load_model - Memory allocated.\n");

    // Reset file pointer to beginning for reading elements
    rewind(file); // Go back to the start of the file

    printf("DEBUG: load_model - Reading elements...\n");
    read_success = read_elements(model, file); // Read into the allocated memory
    if (!read_success) {
        fprintf(stderr, "ERROR: Failed to read elements from model file.\n");
        fclose(file);
        free_model(model); // Free allocated memory if reading fails
        return FALSE;
    }
    printf("DEBUG: load_model - Elements read.\n");

    fclose(file); // Close the file on success
    printf("DEBUG: load_model('%s') - SUCCESS\n", filename);
    return TRUE;
}


// count_elements: Reads file once to count items
void count_elements(Model* model, FILE* file)
{
    char line[LINE_BUFFER_SIZE];
    printf("DEBUG: count_elements - START\n");

    // Reset counts (init_model already does this, but explicit reset here is ok)
    model->n_vertices = 0;
    model->n_texture_vertices = 0;
    model->n_normals = 0;
    model->n_triangles = 0;

    while (fgets(line, LINE_BUFFER_SIZE, file) != NULL) {
        switch (calc_element_type(line)) {
        case VERTEX:         model->n_vertices++;         break;
        case TEXTURE_VERTEX: model->n_texture_vertices++; break;
        case NORMAL:         model->n_normals++;          break;
        case FACE:           model->n_triangles++;        break;
        case NONE: default:                               break; // Ignore others
        }
    }
     printf("DEBUG: count_elements - END\n");
     // No need to reset file pointer here, load_model will handle it
     // fseek(file, original_pos, SEEK_SET);
}


// read_elements: Reads file again to fill allocated arrays
int read_elements(Model* model, FILE* file)
{
    char line[LINE_BUFFER_SIZE];
    int vertex_index = 0;     // Start indices at 0
    int texture_index = 0;
    int normal_index = 0;
    int triangle_index = 0;
    int success;

    printf("DEBUG: read_elements - START\n");

    // --- DO NOT allocate_model(model); HERE --- // REMOVED

    while (fgets(line, LINE_BUFFER_SIZE, file) != NULL) {
        ElementType element_type = calc_element_type(line);
        switch (element_type) {
        case VERTEX:
            if (vertex_index < model->n_vertices) {
                // Pass pointer to the CURRENT index
                success = read_vertex(&(model->vertices[vertex_index]), line);
                if (!success) {
                    fprintf(stderr, "ERROR: Unable to read vertex %d!\n", vertex_index);
                    return FALSE;
                }
                vertex_index++; // Increment AFTER successful read
            } // else { fprintf(stderr, "Warning: Extra vertex found.\n"); }
            break;

        case TEXTURE_VERTEX:
             if (texture_index < model->n_texture_vertices) {
                success = read_texture_vertex(&(model->texture_vertices[texture_index]), line);
                if (!success) {
                    fprintf(stderr, "ERROR: Unable to read texture vertex %d!\n", texture_index);
                    return FALSE;
                }
                texture_index++;
             } // else { fprintf(stderr, "Warning: Extra texture vertex found.\n"); }
            break;

        case NORMAL:
            if (normal_index < model->n_normals) {
                success = read_normal(&(model->normals[normal_index]), line);
                if (!success) {
                    fprintf(stderr, "ERROR: Unable to read normal vector %d!\n", normal_index);
                    return FALSE;
                }
                normal_index++;
            } // else { fprintf(stderr, "Warning: Extra normal found.\n"); }
            break;

        case FACE:
             if (triangle_index < model->n_triangles) {
                success = read_triangle(&(model->triangles[triangle_index]), line);
                if (!success) {
                    fprintf(stderr, "ERROR: Unable to read triangle face %d!\n", triangle_index);
                    return FALSE;
                }
                triangle_index++;
             } // else { fprintf(stderr, "Warning: Extra face found.\n"); }
            break;

        case NONE: default: // Ignore comments, materials, groups, etc.
            break;
        }
    }

     printf("DEBUG: read_elements - END (V=%d, VT=%d, VN=%d, F=%d read)\n",
        vertex_index, texture_index, normal_index, triangle_index);

    // Optional: Check if counts match exactly
    if (vertex_index != model->n_vertices ||
        (model->n_texture_vertices > 0 && texture_index != model->n_texture_vertices) || // Only check if expected
        (model->n_normals > 0 && normal_index != model->n_normals) || // Only check if expected
        triangle_index != model->n_triangles) {
        fprintf(stderr, "WARNING: Mismatch between counted elements and elements read!\n");
        // Consider if this should be a fatal error (return FALSE)
    }

    return TRUE; // Success
}


// calc_element_type: Simple prefix checker
ElementType calc_element_type(const char* text)
{
    if (strncmp(text, "v ", 2) == 0) return VERTEX;
    if (strncmp(text, "vn ", 3) == 0) return NORMAL;
    if (strncmp(text, "vt ", 3) == 0) return TEXTURE_VERTEX;
    if (strncmp(text, "f ", 2) == 0) return FACE;
    // Add other types like 's', 'o', 'g', 'mtllib', 'usemtl' if needed, returning NONE for now
    return NONE; // Default for comments, materials, groups, smoothing, etc.
}


// is_numeric: Basic check for parsing start
int is_numeric(char c)
{
    // Allow digits, sign, and decimal point
    return ((c >= '0' && c <= '9') || c == '-' || c == '.');
}


// --- Parsing functions (keep original logic, add logging if needed) ---

int read_vertex(Vertex* vertex, const char* text)
{
    // Expects "v x y z [w]" format
    // Use sscanf for potentially easier parsing
    int items_read = sscanf(text, "v %lf %lf %lf", &vertex->x, &vertex->y, &vertex->z);
    if (items_read == 3) {
        return TRUE;
    } else {
        // fprintf(stderr, "ERROR: Failed to parse vertex line: %s\n", text); // Can be noisy
        return FALSE;
    }
    // Original parsing logic (kept for reference, sscanf is often simpler)
    /*
    int i = 0;
    while (text[i] != 0 && text[i] != 'v') { ++i; } // Skip 'v'
    ++i;
    while (text[i] != 0 && !is_numeric(text[i])) { ++i; } // Find start of x
    if (text[i] == 0 || sscanf(&text[i], "%lf", &vertex->x) != 1) return FALSE; // Parse x
    while (text[i] != 0 && text[i] != ' ') { ++i; } // Skip x value
    while (text[i] != 0 && !is_numeric(text[i])) { ++i; } // Find start of y
    if (text[i] == 0 || sscanf(&text[i], "%lf", &vertex->y) != 1) return FALSE; // Parse y
    while (text[i] != 0 && text[i] != ' ') { ++i; } // Skip y value
     while (text[i] != 0 && !is_numeric(text[i])) { ++i; } // Find start of z
    if (text[i] == 0 || sscanf(&text[i], "%lf", &vertex->z) != 1) return FALSE; // Parse z
    return TRUE;
    */
}

int read_texture_vertex(TextureVertex* texture_vertex, const char* text)
{
    // Expects "vt u v [w]" format
    int items_read = sscanf(text, "vt %lf %lf", &texture_vertex->u, &texture_vertex->v);
     if (items_read == 2) {
        return TRUE;
    } else {
        // fprintf(stderr, "ERROR: Failed to parse texture vertex line: %s\n", text);
        return FALSE;
    }
}

int read_normal(Vertex* normal, const char* text)
{
     // Expects "vn x y z" format
    int items_read = sscanf(text, "vn %lf %lf %lf", &normal->x, &normal->y, &normal->z);
     if (items_read == 3) {
        return TRUE;
    } else {
        // fprintf(stderr, "ERROR: Failed to parse normal line: %s\n", text);
        return FALSE;
    }
}

int read_triangle(Triangle* triangle, const char* text)
{
    // Expects "f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3" format
    // Or other variations like v1//vn1 or v1/vt1 - this code assumes v/vt/vn
    int items_read = sscanf(text, "f %d/%d/%d %d/%d/%d %d/%d/%d",
                            &triangle->points[0].vertex_index,
                            &triangle->points[0].texture_index,
                            &triangle->points[0].normal_index,
                            &triangle->points[1].vertex_index,
                            &triangle->points[1].texture_index,
                            &triangle->points[1].normal_index,
                            &triangle->points[2].vertex_index,
                            &triangle->points[2].texture_index,
                            &triangle->points[2].normal_index
                            );

    if (items_read == 9) {
        return TRUE;
    } else {
         // Add support for other face formats (v//vn, v/vt) if needed by parsing differently
         fprintf(stderr, "ERROR: Failed to parse face line (expected v/vt/vn format): %s\n", text);
        return FALSE;
    }
     // Keep original manual parsing if preferred, but sscanf is less error-prone
     /*
     int point_index;
     int i = 0;
     // Skip 'f' and spaces
     while (text[i] != 0 && text[i] != 'f') { ++i; }
     ++i;
     while (text[i] != 0 && (text[i] == ' ' || text[i] == '\t')) { ++i; }

     for (point_index = 0; point_index < 3; ++point_index) {
         if (text[i] == 0 || sscanf(&text[i], "%d", &triangle->points[point_index].vertex_index) != 1) return FALSE;
         while (text[i] != 0 && text[i] != '/') { ++i; } // Move to '/' after vertex index
         if (text[i] == 0) return FALSE; // Expected '/'
         ++i; // Move past '/'

         if (text[i] != '/') { // Check if texture index exists
              if (text[i] == 0 || sscanf(&text[i], "%d", &triangle->points[point_index].texture_index) != 1) return FALSE;
              while (text[i] != 0 && text[i] != '/') { ++i; } // Move to '/' after texture index
         } else {
              triangle->points[point_index].texture_index = INVALID_VERTEX_INDEX; // Or 0? Indicate missing.
         }
         if (text[i] == 0) return FALSE; // Expected '/'
         ++i; // Move past '/'

         if (text[i] == 0 || sscanf(&text[i], "%d", &triangle->points[point_index].normal_index) != 1) return FALSE;
         while (text[i] != 0 && text[i] != ' ' && text[i] != '\t' && text[i] != '\n' && text[i] != '\r') { ++i; } // Move to next space/end
          while (text[i] != 0 && (text[i] == ' ' || text[i] == '\t')) { ++i; } // Skip whitespace
     }
     return TRUE;
     */
}