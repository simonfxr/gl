
uniform samplerBuffer position_mass_sampler;
uniform float dt;
uniform float D;
uniform float damping;

in vec4 position_mass;
in vec3 velocity;
in ivec4 connection;

out Out1 {
    vec4 position_mass;
} out1;

out Out2 {
    vec3 velocity;
} out2;

void main() {

    vec3 pos = position_mass.xyz;
    float inv_mass = position_mass.w;
    vec3 v = velocity;

    vec3 F = vec3(0);

    for (int i = 0; i < 4; ++i) {
        vec3 r = texelFetch(position_mass_sampler, connection[i]).xyz;
        vec3 s = r - pos;
        F += D * (length(s) - 1.0) * normalize(s);
    }

    vec3 a = inv_mass == 0 ? vec3(0) : F * inv_mass + vec3(0, -9.81, 0);

    pos += dt * v + (0.5 * dt * dt) * a;
    v = (v + dt * a) * damping;
    
    out1.position_mass = vec4(pos, inv_mass);
    out2.velocity = v;
}
