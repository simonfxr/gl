
uniform sampler2D texture0;
uniform float kernel[16];

in vec2 texCoord;
out vec4 color;

const int N = 19;
const float N2 = (N - 1) * 0.5;

void main() {

    vec4 samp = vec4(0);
    vec2 texelSize = vec2(1) / (textureSize(texture0, 0) - vec2(1));

    for (int i = 0; i < N; ++i) {
        float y = float(i) - N2;
        samp += kernel[i] * texture(texture0, texCoord + vec2(0, y) * texelSize);
    }
    
    color = samp;
}
