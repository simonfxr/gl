uniform float fade_factor;
uniform sampler2D textures[2];

in vec2 texcoord;
out vec4 FragColor;


void main()
{
    FragColor = mix(
        texture2D(textures[0], texcoord),
        texture2D(textures[1], texcoord),
        fade_factor
    );
}
