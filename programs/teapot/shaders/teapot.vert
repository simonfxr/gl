

uniform mat4 projectionMatrix;
uniform mat4 mvMatrix;
uniform mat3 normalMatrix;

in vec3 position;
in vec3 normal;

out vec3 ecNormal;
out vec3 ecPosition;

void
main()
{
    ecNormal = normalMatrix * normal;
    vec4 ecVert = mvMatrix * vec4(position, 1.0);
    ecPosition = vec3(ecVert) / ecVert.w;
    gl_Position = projectionMatrix * vec4(ecPosition, 1.0);
}
