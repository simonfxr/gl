#version 330

/* uniform vec3 BrickColor, MortarColor; */
/* uniform vec2 BrickSize; */
/* uniform vec2 BrickPct; */

const vec3 BrickColor = vec3(1, 0, 0);
const vec3 MortarColor = vec3(0.6);
const vec2 BrickSize = vec2(1, 1);
const vec2 BrickPct = vec2(0.9);

in vec2 position;
in float lightIntensity;

out vec4 FragColor;

void main()
{
    vec3 color;
    vec2 p, useBrick;

    p = position / BrickSize;
    if (fract(p.y * 0.5) > 0.5)
        p.x += 0.5;
    
    p = fract(p);
    useBrick = step(p, BrickPct);
    
    color = mix(MortarColor, BrickColor, useBrick.x * useBrick.y);
    color *= lightIntensity;
    FragColor = vec4(color, 1.0);
}
