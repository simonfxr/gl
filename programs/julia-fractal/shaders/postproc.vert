
SL_in vec2 position;

SL_out vec2 texCoord;

void
main()
{
    texCoord = (position + vec2(1)) * 0.5;
    gl_Position = vec4(position, 0, 1);
}
