
uniform float depth;

in vec3 position;

out vec3 vTexCoord;

void main() {
    gl_Position = vec4(position, 1);
    vTexCoord = vec3(position.xy * 0.5 + vec2(0.5), depth);
}
