uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;
uniform mat3 normalMatrix;

SL_in vec4 position;
SL_in vec3 normal;

SL_out vec3 ecPosition;
SL_out vec3 ecNormal;

void
main()
{
    ecPosition = vec3(mvMatrix * position);
    ecNormal = normalMatrix * normal;
    gl_Position = mvpMatrix * position;
}
