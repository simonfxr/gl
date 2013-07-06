
uniform samplerBuffer particle_data1;

uniform mat4 vpMatrix;
uniform float grid_size;

in vec3 position;

void main() {
    vec3 offset = texelFetch(particle_data1, gl_InstanceID).xyz;
    
    gl_Position = vpMatrix * vec4((position + offset) * (1 / grid_size), 1);
}
