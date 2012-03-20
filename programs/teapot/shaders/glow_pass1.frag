
uniform sampler2D texture0;

in vec2 texCoord;

out vec4 color;

void main() {

    vec4 samp = vec4(0);
    vec2 texelSize = vec2(1) / (textureSize(texture0, 0) - vec3(1));
    int n = 5;
    float n2 = float(n - 1) / 2;
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            samp += texture(texture0, texCoord + (vec2(ivec2(i , j)) - vec2(n2)) * texelSize);
    samp /= float(n * n);

    color = samp;
}
