

uniform sampler1D instanceData;
uniform mat3 normalMatrix;
uniform mat4 pMatrix;
uniform mat4 vMatrix;

in vec4 position;
in vec3 normal;

out vec3 ecPosition;
out vec3 ecNormal;
flat out int instanceID;
/* flat out vec3 color; */
/* flat out float shininess; */

void main() {
    vec4 data1 = texelFetch(instanceData, gl_InstanceID * 2, 0);
    vec3 offset = data1.xyz;
    float rad = data1.w;
    /* vec4 data2 = texelFetch(instanceData, gl_InstanceID * 2 + 1, 0); */
    /* color = data2.rgb; */
    /* shininess = data2.a; */

    vec4 ecPos4 = vMatrix * vec4(vec3(position) * rad + offset, 1.0);
    ecPosition = vec3(ecPos4);
    ecNormal = normalMatrix * normal;
    gl_Position = pMatrix * ecPos4;
    instanceID = gl_InstanceID;
}
