
vec4 position;
vec3 normal;

out vec2 texCoord;

void main() {
    texCoord = position.st;
    gl_Position = position;
}
