#version 330 core
layout(location = 0) in vec3 position;

uniform mat4 gModel;      // positions/rotates the object in world space
uniform mat4 gView;       // camera position and orientation
uniform mat4 gProjection; // near and far

void main() {
    gl_Position = gProjection * gView * gModel * vec4(position, 1.0);
}
