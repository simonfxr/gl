uniform sampler3D worldVolume;

in vec3 gTexCoord;
in bool dead;
out vec4 fColor;

void main() {
    float val = texture(worldVolume, gTexCoord).r;

    if (!dead) {
        fColor = vec4(1, 0, 0, 1);
    } else {
        fColor = vec4(vec3(0.5), 1);
    }
}
