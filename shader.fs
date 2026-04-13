#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3      objectColor;
uniform sampler2D gTexture;
uniform bool      useTexture;
uniform bool      useShine;
uniform vec3      viewPos;

// multiple lights
#define MAX_LIGHTS 10
uniform int       numLights;
uniform vec3      lightPos[MAX_LIGHTS];
uniform vec3      lightColor[MAX_LIGHTS];

out vec4 FragColor;

void main() {
    vec3 baseColor = useTexture ? texture(gTexture, TexCoord).rgb : objectColor;
    vec3 norm      = normalize(Normal);
    vec3 viewDir   = normalize(viewPos - FragPos);

    vec3 ambient = 0.15 * baseColor;
    vec3 result  = ambient;

    for (int i = 0; i < numLights; i++) {
        vec3 lightDir   = normalize(lightPos[i] - FragPos);

        float diff      = max(dot(norm, lightDir), 0.0);
        vec3  diffuse   = diff * baseColor * lightColor[i];

        // specular
        vec3  reflDir   = reflect(-lightDir, norm);
        float spec      = useShine ? pow(max(dot(viewDir, reflDir), 0.0), 256.0) : 0.0;
        vec3  specular  = 0.6 * spec * lightColor[i];

        result += diffuse + specular;
    }

    FragColor = vec4(result, 1.0);
}
