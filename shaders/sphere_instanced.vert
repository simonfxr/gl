#version 330

uniform sampler1D instancesMap;
uniform mat3 normalMatrix;
uniform mat4 pMatrix;

in vec4 position;
in vec3 normal;

out vec3 ecPosition;
out vec3 ecNormal;

void main() {
    vec4 instance = texelFetch(instancesMap, gl_InstanceID, 0);
    float rad = instance.w;
    vec3 offset = instance.xyz;
    
    ecPosition = vec3(position) * rad + offset;
    ecNormal = normalMatrix * normal;
    gl_Position = pMatrix * vec4(ecPosition, 1);
}
