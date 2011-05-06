

uniform mat4 mvpMatrix;

in vec4 position;
in vec3 normal;

out vec2 texCoord;

void main() {
    texCoord = position.st;
    gl_Position = mvpMatrix * position;
}
