in vec2 position;
out vec2 worldPosition;

void main() {
    worldPosition = position;
    gl_Position = vec4(position, 0, 1);
}
