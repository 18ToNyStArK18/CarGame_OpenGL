#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 aNormal; // we get the normal of every vertex
layout(location = 2) in vec2 aUV;       

uniform mat4 gModel;
uniform mat4 gView;
uniform mat4 gProjection;

out vec2 TexCoord;
out vec3 FragPos;    
out vec3 Normal;     

void main() {
    vec4 worldPos = gModel * vec4(position, 1.0);
    FragPos   = worldPos.xyz;
    Normal    = mat3(transpose(inverse(gModel))) * aNormal;
    TexCoord  = aUV;
    gl_Position = gProjection * gView * worldPos;
}

