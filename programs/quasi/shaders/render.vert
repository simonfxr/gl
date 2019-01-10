SL_in vec2 position;
SL_out vec2 worldPosition;

void
main()
{
    worldPosition = position;
    gl_Position = vec4(position, 0, 1);
}
