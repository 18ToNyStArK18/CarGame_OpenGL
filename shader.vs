#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 aUV;       // UV from separate VBO

uniform mat4 gModel;
uniform mat4 gView;
uniform mat4 gProjection;

out vec2 TexCoord;

void main() {
    gl_Position = gProjection * gView * gModel * vec4(position, 1.0);
    TexCoord = aUV;
}

