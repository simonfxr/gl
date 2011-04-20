#version 330

in vec4 position;
in vec3 normal;

out vec2 texCoord;

void main() {
    gl_Position = position;
    texCoord = position.st * 0.5 + vec2(0.5f);
}
