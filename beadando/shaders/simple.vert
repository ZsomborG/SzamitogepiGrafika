#version 330 core

layout (location = 0) in vec3 aPos;      // Vertex position (model space)
layout (location = 1) in vec3 aNormal;   // Vertex normal (model space)
layout (location = 2) in vec2 aTexCoord; // Texture coordinate

// Outputs to Fragment Shader
out vec3 FragPos_world;   // Vertex position in world space
out vec3 FragNormal_world; // Normal in world space
out vec2 TexCoord;

// Uniforms (values set from C++ code)
uniform mat4 model;      // Model transformation matrix
uniform mat4 view;       // View transformation matrix
uniform mat4 projection; // Projection transformation matrix

void main()
{
    FragPos_world = vec3(model * vec4(aPos, 1.0));

    FragNormal_world = normalize(mat3(transpose(inverse(model))) * aNormal);
    if (length(FragNormal_world) < 0.001) {
        FragNormal_world = vec3(0.0, 1.0, 0.0); // Default to up if it's zero
    }
    // Calculate final position in clip space
    gl_Position = projection * view * model * vec4(aPos, 1.0);

    // Pass texture coordinate to fragment shader
    TexCoord = aTexCoord;
}