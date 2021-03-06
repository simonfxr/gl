
in vec3 position;
in vec3 normal;

out vec2 texCoord;

void
main()
{
    texCoord = position.xy;
    gl_Position = vec4(position * 2 - vec3(1), 1);
}
