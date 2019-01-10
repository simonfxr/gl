
in vec3 ecNormal;
in vec3 ecPosition;
in vec3 vertColor;

out vec4 color;

void
main()
{
    vec3 N = normalize(ecNormal);
    vec3 C = vertColor;
    vec3 V = -normalize(ecPosition);
    float D = dot(N, V);
    vec3 shadedColor = (max(dot(N, C), 0) / dot(C, C)) * C;
    color = vec4(0.7 * C + 0.3 * shadedColor, 1);
    if (D < 0)
        color = vec4(0, 0, 0, 1);
}
