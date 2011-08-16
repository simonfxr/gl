uniform uint layers;

layout(points) in;

in vec2 vTexCoord[1];

layout(points, max_vertices = 33) out;

out vec3 gTexCoord;

void main() {
    float scale = 1 / (layers - 1);
    for (uint i = 0; i < layers; ++i) {
        gl_Layer = int(i);
        gl_Position = gl_in[0].gl_Position;
        gTexCoord = vec3(vTexCoord[0], i * scale);
        EmitVertex();
    }
}
