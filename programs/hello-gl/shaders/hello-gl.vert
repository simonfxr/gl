
SL_in vec2 position;
SL_out vec2 texcoord;

void
main()
{
    gl_Position = vec4(position, 0.0, 1.0);
    texcoord = position * vec2(0.5) + vec2(0.5);
}
