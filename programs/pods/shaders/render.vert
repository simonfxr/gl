uniform float time;

SL_in vec2 position;
SL_out vec2 uv;

void
main()
{
    uv = position * 0.5 + 0.5;
    gl_Position = vec4(position, 0, 1);
}
