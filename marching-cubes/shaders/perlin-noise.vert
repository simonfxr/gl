in vec3 position;

out vec2 vTexCoord;

void main() {
    gl_Position = vec4(position, 1.0);
    vTexCoord = position.xy * 0.5 + vec2(0.5);
}
