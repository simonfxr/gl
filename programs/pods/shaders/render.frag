uniform sampler2D world;

SL_in vec2 uv;

SL_out vec4 fragColor;

void
main()
{
    fragColor = vec4(texture(world, uv).rgb, 1);
}
