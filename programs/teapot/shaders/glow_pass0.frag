
uniform sampler2DMS texture0;

in vec2 texCoord;
out vec4 color;

void main() {

    ivec2 index = ivec2(texCoord * textureSize(texture0));
    
    int n = 4;
    vec4 samp = vec4(0);
    for (int i = 0; i < n; ++i)
        samp += texelFetch(texture0, index, i);
    samp /= float(n);

    color = vec4((0.5 * samp.rgb + 0.5 * vec3(1)) * samp.a, samp.a);
}
