#version 330

const vec3 BrickColor  = vec3(1, 0, 0);
const vec3 MortarColor = vec3(0.6);
const vec2 BrickSize   = vec2(0.06, 0.03);
const vec2 BrickPct    = vec2(0.9, 0.85);

in float lightIntensity;
in vec2  vPosition;

out vec4 fragColor;

void main()
{
    vec3 color;
    vec2 position, useBrick;

    position = vPosition / BrickSize;

    if (fract(position.y * 0.5) > 0.5)
        position.x += 0.5;

    position = fract(position);

    useBrick = step(position, BrickPct);

    color = mix(MortarColor, BrickColor, useBrick.x * useBrick.y);
    color *= lightIntensity;
    fragColor = vec4(color, 1);
}
