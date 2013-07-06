
uniform samplerBuffer particle_data1;

uniform mat4 mvpMatrix;

in vec3 position;

void main() {
    vec3 offset = texelFetch(particle_data1, gl_InstanceID).xyz;
    gl_Position = mvpMatrix * vec4(position + offset, 1);
}
