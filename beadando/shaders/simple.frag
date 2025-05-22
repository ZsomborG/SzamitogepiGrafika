#version 330 core

in vec3 FragPos_world;
in vec3 FragNormal_world;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D texture1;         // Name: texture1
uniform vec4 uColorTint;          // Name: uColorTint

uniform vec3 lightDir_world;      // Name: lightDir_world
uniform vec3 lightColor;          // Name: lightColor
uniform vec3 ambientLightColor;   // Name: ambientLightColor

uniform vec3 materialDiffuseColor; // Name: materialDiffuseColor
uniform vec3 materialSpecularColor;// Name: materialSpecularColor
uniform float materialShininess;   // Name: materialShininess

uniform vec3 viewPos_world;       // Name: viewPos_world

void main()
{
    vec4 texColor = texture(texture1, TexCoord) * uColorTint; // Uses texture1, uColorTint

    vec3 norm = normalize(FragNormal_world);
    vec3 lightDirection = normalize(lightDir_world);         // Uses lightDir_world

    vec3 ambient = ambientLightColor * materialDiffuseColor; // Uses ambientLightColor, materialDiffuseColor

    float diff = max(dot(norm, lightDirection), 0.0);
    vec3 diffuse = diff * lightColor * materialDiffuseColor; // Uses lightColor, materialDiffuseColor

    vec3 viewDir = normalize(viewPos_world - FragPos_world); // Uses viewPos_world
    vec3 reflectDir = reflect(-lightDirection, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), materialShininess); // Uses materialShininess
    vec3 specular = spec * lightColor * materialSpecularColor; // Uses lightColor, materialSpecularColor

    vec3 lightingResult = ambient + diffuse + specular;
    FragColor = vec4(lightingResult * texColor.rgb, texColor.a);
}