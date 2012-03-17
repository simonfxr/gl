
uniform sampler2DMS texture0;
uniform sampler2D texture1;

in vec2 texCoord;
out vec4 color;

void main() {
    ivec2 index = ivec2(textureSize(texture0) * texCoord);

    vec4 sum = vec4(0);
    int n = 4;
    for (int i = 0; i < n; ++i)
        sum += texelFetch(texture0, index, i);

    vec3 glow = texture(texture1, texCoord).rgb;

    sum.rgb /= float(n);
    color = vec4(glow + sum.rgb, 1);
    color.rgb = clamp(color.rgb, vec3(0), vec3(1));
}
