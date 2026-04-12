#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3      objectColor;
uniform sampler2D gTexture;
uniform bool      useTexture;
uniform vec3      lightPos;    
uniform vec3      viewPos;     
uniform bool      useShine;

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


    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 256);
    vec3 specular = specularStrength * spec *  vec3(1.0);

    if(useShine)
        FragColor = vec4(ambient + diffuse + specular, 1.0);
    else
        FragColor = vec4(ambient + diffuse, 1.0);

}
