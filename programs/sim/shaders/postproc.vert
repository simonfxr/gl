SL_in vec4 position;
SL_in vec3 normal;

SL_out vec2 texCoord;

void
main()
{
    gl_Position = position;
    texCoord = vec2(0.5) + 0.5 * position.xy;
}
