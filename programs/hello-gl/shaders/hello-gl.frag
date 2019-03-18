
uniform float fade_factor;
uniform sampler2D texture0;
uniform sampler2D texture1;

SL_in vec2 texcoord;

#if __VERSION__ >= 130
out vec4 FragColor;
#else
#    define FragColor gl_FragColor
#endif

void
main()
{
    vec3 color = mix(texture2D(texture0, texcoord).xyz,
                     texture2D(texture1, texcoord).xyz,
                     fade_factor);
    FragColor = vec4(color, 1.);
}
