
uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D texture3;
uniform sampler2D texture4;
uniform sampler2D texture5;

in vec2 texCoord;

out vec4 color;

void main() {
    color = texture(texture0, texCoord);
}
