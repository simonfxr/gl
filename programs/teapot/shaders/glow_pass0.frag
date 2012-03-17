
uniform sampler2DMS texture0;

in vec2 texCoord;
out vec4 color;

void main() {

    ivec2 index = ivec2(texCoord * textureSize(texture0));
    
    int n = 4;
    vec4 sample = vec4(0);
    for (int i = 0; i < n; ++i)
        sample += texelFetch(texture0, index, i);
    sample /= float(n);

    color = vec4((0.5 * sample.rgb + 0.5 * vec3(1)) * sample.a, sample.a);
}
