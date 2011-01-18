#version 330

uniform sampler2D mirrorTexture;

in vec2 texCoord;
out vec4 color;

void main() {
    color = texture(mirrorTexture, texCoord);
}
