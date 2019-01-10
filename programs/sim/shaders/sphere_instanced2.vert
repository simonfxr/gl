
uniform mat4 pMatrix;

SL_in vec4 position;
SL_in vec3 normal;
SL_in vec4 colorShininess;
SL_in mat4 mvMatrix;

SL_out vec3 ecPosition;
SL_out vec3 ecNormal;
SL_out vec3 color;
SL_out float shininess;

void
main()
{
    vec4 ecPos4 = mvMatrix * position;
    ecPosition = vec3(ecPos4);
    ecNormal = normalize((mvMatrix * vec4(normal, 0.)).xyz);
    gl_Position = pMatrix * ecPos4;
    color = colorShininess.rgb;
    shininess = colorShininess.a;
}
