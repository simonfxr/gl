
in vec4 position_mass;

out Vertex {
    vec4 position_mass;
};

void main() {
    Vertex.position_mass = position_mass;
}
