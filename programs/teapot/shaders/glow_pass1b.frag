
uniform sampler2D texture0;

in vec2 texCoord;
out vec4 color;

const int N = 16;
const float SIG = 0.84089642;
const float SIG2 = SIG * SIG;
const float SQRT_PI = 1.7724538509055;
const float SQRT_2  = 1.4142135623730;

float weight(float x) {
    return 1/(SQRT_PI * SQRT_2 * SIG) * exp(-1/(2 * SIG2) * (x * x));
}

void main() {

    vec4 samp = vec4(0);
    vec2 texelSize = vec2(1) / (textureSize(texture0, 0) - vec2(1));
    const float N2 = float(N - 1) / 2;
    for (int i = 0; i < N; ++i) {
        float y = float(i) - N2;
        samp += weight(y) * texture(texture0, texCoord + vec2(0, y) * texelSize);
    }
    
    color = samp;
}
