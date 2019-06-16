
uniform float time;
uniform vec2 julia_constant;
uniform vec2 world_shift;
uniform float world_zoom;

SL_in vec2 position;

SL_out vec2 worldPosition;
SL_flat SL_out float zoom;
SL_flat SL_out vec2 shift;
SL_flat SL_out vec2 C;

void
main()
{
    worldPosition = position;
    gl_Position = vec4(position, 0, 1);

    zoom = world_zoom;
    shift = world_shift;
    float t = time + 1.47;
    C = vec2(sin(0.1 * t), 0.3 * cos(0.19 * t));

    //     // float zooom = (1 + sin(0.05 * time - 1.57 ));
    //     // zooom = 1 + 5 * zoom;

    //     zoom = vec2(1, 1);

    //     float time0 = time * 0.2;
    //     float T = time0 * 0.3;
    //     float a = 1.1 * cos(T) + 0.5 * cos(1.3 * T) - 5 * sin(0.1 * T);
    //     float b = 0.7 * sin(1.7 * T) - 0.8 * sin(0.7 * T) + 5 * sin(0.1 * T);

    //     C = 0.5 * vec2(sin(time0), cos(time0)) + 0.4 * vec2(0.219 * (1 +
    //     sin(time)));

    //     float l = length(C);
    //     vec2 C1 = normalize(C);
    //     vec2 C2 = vec2(C1.y, -C1.x);

    // //    C += -0.3 * sin(0.3 * T + 1.57) * C1 + 0.3 * C2 * cos(0.3 * T
    // + 1.57);
    //     ab = vec2(a, b);
    //     shift = vec2(0, 0);
}
