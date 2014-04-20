#include "nbody_phys.hpp"
#include "err/err.hpp"

ParticleRef::ParticleRef(ParticleArray& a, defs::index _i) :
    array(a), i(_i) {}

ParticleArray::ParticleArray(defs::size s) :
    _n(0), _size(s),
    _position(new point3_t[UNSIZE(s)]),
    _velocity(new vec3_t[UNSIZE(s)]),
    _inv_mass(new real[UNSIZE(s)]),
    _charge(new real[UNSIZE(s)])
{}

ParticleArray::~ParticleArray() {
    delete[] _position;
    delete[] _velocity;
    delete[] _inv_mass;
    delete[] _charge;
}

Particle ParticleArray::operator[](index i) const {
    Particle p;
    p.position = _position[i];
    p.velocity = _velocity[i];
    p.inv_mass = _inv_mass[i];
    p.charge = _charge[i];
    return p;
}

ParticleRef ParticleArray::operator[](index i) {
    return ParticleRef(*this, i);
}

namespace {

template <typename T>
T *resize(T *arr, size old_size, size new_size) {
    T *brr = new T[UNSIZE(new_size)];
    size k = old_size < new_size ? old_size : new_size;
    for (index i = 0; i < k; ++i)
        brr[i] = arr[i];
    delete[] arr;
    return brr;
}

} // namespace anon

void ParticleArray::push_back(const Particle& p) {
    if (_n >= _size) {
        defs::size old_size = _size;
        _size = old_size < 4 ? 8 : old_size * 2;

        _position = resize(_position, old_size, _size);
        _velocity = resize(_velocity, old_size, _size);
        _inv_mass = resize(_inv_mass, old_size, _size);
        _charge = resize(_charge, old_size, _size);
    }

    _n++;
    put(_n - 1, p);
}

void ParticleArray::put(index i, const Particle& p) {
    _position[i] = p.position;
    _velocity[i] = p.velocity;
    _inv_mass[i] = p.inv_mass;
    _charge[i] = p.charge;
}

ParticleRef::operator Particle() const {
    const ParticleArray& arr = array;
    return arr[i];
}

ParticleRef& ParticleRef::operator =(const Particle& p) {
    array.put(i, p);
    return *this;
}

Simulation::Simulation(real _time_step) :
    time_step(_time_step),
    particles(8)
{}

Simulation::~Simulation() {}

void Simulation::init() {}

void Simulation::compute_acceleration(vec3_t *acceleration, const vec3_t *position, const vec3_t *velocity) {

    const real epsi0 = real(8.85e-2);
    const real mu0 = real(4 * math::PI * 1e2);

    for (defs::index i = 0; i < particles._n; ++i) {
        Particle a = particles[i];
        vec3_t E = vec3(real(0));
        vec3_t B = vec3(real(0));
        
        for (defs::index j = 0; j < particles._n; ++j) {
            if (i == j) continue;
            const Particle b = particles[j];

            vec3_t r = position[i] - position[j];
            real r2 = dot(r, r);
            vec3_t n = normalize(r);

            E += inverse(4 * math::PI * epsi0 * r2) * b.charge * n;
            B += mu0 * b.charge * inverse(4 * math::PI * r2) * cross(velocity[j], n);
        }

        acceleration[i] = a.inv_mass * a.charge * (E + cross(velocity[i], B));
    }
}

void Simulation::simulate_frame() {
    const real dt = time_step;
    const real dt2 = real(.5) * dt;

    vec3_t *accel = new vec3_t[particles._n];
    vec3_t *accel1 = new vec3_t[particles._n];
    vec3_t *pos = new vec3_t[particles._n];
    vec3_t *vel = new vec3_t[particles._n];

    compute_acceleration(accel, particles._position, particles._velocity);
    for (defs::index i = 0; i < particles._n; ++i) {
        pos[i] = particles._position[i] + dt2 * particles._velocity[i];
        vel[i] = particles._velocity[i] + dt2 * accel[i];
        accel[i] *= real(1) / real(6);
    }

    compute_acceleration(accel1, pos, vel);
    for (defs::index i = 0; i < particles._n; ++i) {
        pos[i] = particles._position[i] + dt2 * vel[i];
        vel[i] = particles._velocity[i] + dt2 * accel1[i];
        accel[i] += (real(2) / real(6)) * accel1[i];
    }

    compute_acceleration(accel1, pos, vel);
    for (defs::index i = 0; i < particles._n; ++i) {
        pos[i] = particles._position[i] + dt * vel[i];
        vel[i] = particles._velocity[i] + dt * accel1[i];
        accel[i] += (real(2) / real(6)) * accel1[i];
    }
    
    compute_acceleration(accel1, pos, vel);
    for (defs::index i = 0; i < particles._n; ++i) {
        vec3_t a = accel[i] + (real(1) / real(6)) * accel1[i];
        particles._position[i] += dt * particles._velocity[i] + real(.5) * dt * dt * a;
        particles._velocity[i] += dt * a;
    }

    delete[] accel;
    delete[] accel1;
    delete[] pos;
    delete[] vel;
}

void Simulation::extrapolate_particle(Particle& p, float interpolation) {
    p.position += interpolation * time_step * p.velocity;
}
