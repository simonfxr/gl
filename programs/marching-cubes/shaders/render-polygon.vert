uniform mat4 mvMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;

in vec3 position;
in vec3 normal;

out vec3 ecPosition;
out vec3 ecNormal;

void
main()
{
    ecNormal = normalMatrix * position;
    vec4 ecPos4 = mvMatrix * vec4(position, 1);
    ecPosition = ecPos4.xyz;
    gl_Position = projectionMatrix * ecPos4;
}
