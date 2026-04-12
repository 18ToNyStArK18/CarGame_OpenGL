#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3      objectColor;
uniform sampler2D gTexture;
uniform bool      useTexture;
uniform vec3      lightPos;    
uniform vec3      viewPos;     

out vec4 FragColor;

void main() {
    vec3 baseColor = useTexture ? texture(gTexture, TexCoord).rgb
                                : objectColor;

    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * baseColor;

    vec3 norm     = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff    = max(dot(norm, lightDir), 0.0);
    vec3 diffuse  = diff * baseColor;

    FragColor = vec4(ambient + diffuse, 1.0);
}
