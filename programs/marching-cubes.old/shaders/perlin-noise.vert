uniform float depth;

in vec3 position;

out vec3 vTexCoord;

void main() {
    vTexCoord = vec3(position.xy, depth);
    gl_Position = vec4(position * 2 - vec3(1, 1, 0), 1);
}
