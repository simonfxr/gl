
layout(triangles) in;

in vec2 vTexCoord[3];

out vec3 gTexCoord;

#ifdef LAYERED                  

#define LAYERS 33
#define VERTICES 99

layout(triangle_strip, max_vertices = VERTICES) out;

void main() {
    float scale = 1 / (LAYERS - 1);
    for (int i = 0; i < LAYERS; ++i) {
        gl_Layer = i;
        float depth = i * scale;
        for (int j = 0; j < 3; ++j) {
            gl_Position = gl_in[j].gl_Position;
            gTexCoord = vec3(vTexCoord[j], depth);
            EmitVertex();
        }
        EndPrimitive();
    }
}

#else

uniform float depth;

layout(triangle_strip, max_vertices = 3) out;

void main() {
    for (int i = 0; i < 3; ++i) {
        gl_Position = gl_in[i].gl_Position;
        gTexCoord = vec3(vTexCoord[i], depth);
        EmitVertex();
    }
}

#endif
