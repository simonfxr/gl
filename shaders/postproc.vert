#version 330

in vec4 position;
in vec3 normal;

out vec2 texCoord;

void main() {
    gl_Position = position;
    texCoord = position.st;
}
