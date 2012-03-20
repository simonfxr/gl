
uniform sampler2D texture0;

in vec2 texCoord;
out vec4 color;

const int N = 7;
const float SIG = 0.84089642;
const float SIG2 = SIG * SIG;
const float PI = 3.14159265;

float weight(vec2 v) {
    return 1/(PI * 2 * SIG2) * exp(-1/(2 * SIG2) * dot(v, v));
}

void main() {

    vec4 samp = vec4(0);
    vec2 texelSize = vec2(1) / (textureSize(texture0, 0) - vec2(1));
    const float N2 = float(N - 1) / 2;
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            vec2 p = vec2(ivec2(i, j)) - vec2(N2);
            samp += weight(p) * texture(texture0, texCoord + p * texelSize);
        }
    }
    
    color = samp;
}
