
uniform sampler2DMS texture0;

in vec2 texCoord;
out vec4 color;

void main() {
    ivec2 index = ivec2(textureSize(texture0) * texCoord);

    vec4 sum = vec4(0);
    int n = 4;
    for (int i = 0; i < n; ++i)
        sum += texelFetch(texture0, index, i);

    color = sum / float(n);
}
