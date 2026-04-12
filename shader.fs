#version 330 core
in vec2 TexCoord;
 
uniform vec3      objectColor;
uniform sampler2D gTexture;
uniform bool      useTexture;   // true = buildings, false = car/track/NPC
 
out vec4 FragColor;
 
void main() {
    if (useTexture)
        FragColor = texture(gTexture, TexCoord);
    else
        FragColor = vec4(objectColor, 1.0);
}

