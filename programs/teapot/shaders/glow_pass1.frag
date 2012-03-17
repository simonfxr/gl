
uniform sampler2D texture0;

in vec2 texCoord;

out vec4 color;

void main() {

    vec4 sample = vec4(0);
    vec2 texelSize = vec2(1) / textureSize(texture0, 0);
    int n = 7;
    float n2 = float(n - 1) / 2;
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            sample += texture(texture0, texCoord + (vec2(ivec2(i , j)) - vec2(n2)) * texelSize);
    sample /= float(n * n);

    color = sample;
}
