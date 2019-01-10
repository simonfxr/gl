
uniform mat4 mvpMatrix;

in vec3 position;
in vec4 color;

out vec4 fragColor;

void
main()
{
    fragColor = color;
    gl_Position = mvpMatrix * vec4(position, 1.0);
}
