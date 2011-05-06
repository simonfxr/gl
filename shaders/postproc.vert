

in vec4 position;
in vec3 normal;

out vec2 texCoord;

void main() {
    gl_Position = position;
    texCoord = vec2(0.5) + 0.5 * position.xy;
}

