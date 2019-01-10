
uniform float fade_factor;
uniform sampler2D texture0;
uniform sampler2D texture1;

in vec2 texcoord;
out vec4 FragColor;

void
main()
{
    FragColor = mix(texture2D(texture0, texcoord),
                    texture2D(texture1, texcoord),
                    fade_factor);
}
