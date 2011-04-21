#version 330

uniform sampler1D instancesMap;
uniform mat3 normalMatrix;
uniform mat4 pMatrix;
uniform mat4 vMatrix;

in vec4 position;
in vec3 normal;

out vec3 ecPosition;
out vec3 ecNormal;

void main() {
    vec4 instance = texelFetch(instancesMap, gl_InstanceID, 0);
    float rad = instance.w;
    vec3 offset = instance.xyz;

    vec4 ecPos4 = vMatrix * vec4(vec3(position) * rad + offset, 1.0);
    ecPosition = vec3(ecPos4);
    ecNormal = normalMatrix * normal;
    gl_Position = pMatrix * ecPos4;
}
