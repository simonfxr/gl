#include "nbody_phys.hpp"

#include "err/err.hpp"

using namespace math;
using math::sqrt;

inline constexpr double m = double(1e15);
inline constexpr double sec = double(1e12);
inline constexpr double kg = double(1);
inline constexpr double As = double(1);

inline constexpr double epsi0 =
  double(8.854187817e-12) * (As * As * (sec / m) * (sec / m) / (kg * m));
// inline constexpr double c = double(2.99792458e8) * m / sec;

// inline constexpr double mu0 = inverse(c * c * epsi0) * (kg * m / (As * As));

inline constexpr double r_p = double(0.84e-15) * m;
inline constexpr double r_e = double(2.8e-15) * m;

inline constexpr double m_p = double(1.672e-27) * kg;
inline constexpr double m_e = double(9.109382e-31) * kg;

inline constexpr double e0 = double(1.6e-19) * As;

ParticleRef::ParticleRef(ParticleArray &a, uint32_t _i) : array(a), i(_i) {}

ParticleArray::ParticleArray(size_t s)
  : _n(0)
  , _size(s)
  , _position(new point3_t[s])
  , _velocity(new vec3_t[s])
  , _inv_mass(new real[s])
  , _charge(new real[s])
  , _radius(new real[s])
{}

ParticleArray::~ParticleArray()
{
    delete[] _position;
    delete[] _velocity;
    delete[] _inv_mass;
    delete[] _charge;
    delete[] _radius;
}

Particle ParticleArray::operator[](size_t i) const
{
    Particle p;
    p.position = _position[i];
    p.velocity = _velocity[i];
    p.inv_mass = _inv_mass[i];
    p.charge = _charge[i];
    p.radius = _radius[i];
    return p;
}

ParticleRef ParticleArray::operator[](size_t i)
{
    return ParticleRef(*this, i);
}

namespace {

template<typename T>
T *
resize(T *arr, size_t old_size, size_t new_size)
{
    auto brr = new T[new_size];
    auto k = old_size < new_size ? old_size : new_size;
    for (size_t i = 0; i < k; ++i)
        brr[i] = arr[i];
    delete[] arr;
    return brr;
}

} // namespace

void
ParticleArray::push_back(const Particle &p)
{
    if (_n >= _size) {
        size_t old_size = _size;
        _size = old_size < 4 ? 8 : old_size * 2;

        _position = resize(_position, old_size, _size);
        _velocity = resize(_velocity, old_size, _size);
        _inv_mass = resize(_inv_mass, old_size, _size);
        _charge = resize(_charge, old_size, _size);
        _radius = resize(_radius, old_size, _size);
    }

    _n++;
    put(_n - 1, p);
}

void
ParticleArray::put(size_t i, const Particle &p)
{
    _position[i] = p.position;
    _velocity[i] = p.velocity;
    _inv_mass[i] = p.inv_mass;
    _charge[i] = p.charge;
    _radius[i] = p.radius;
}

ParticleRef::operator Particle() const
{
    const ParticleArray &arr = array;
    return arr[i];
}

ParticleRef &
ParticleRef::operator=(const Particle &p)
{
    array.put(i, p);
    return *this;
}

Simulation::Simulation(real _time_step)
  : time_step(_time_step / sec), particles(8)
{}

Simulation::~Simulation() {}

void
Simulation::init()
{

    /*
     * m_e v^2 = e^2 / (4 pi epsi0) 1/r
     * v = sqrt(e^2 / (4 pi epsi0 m_red) 1/r)
     */

#define v(r) math::sqrt(e0 *e0 / (4 * math::PI * epsi0 * m_e) * r)

    Particle e;
    Particle p;

    p.position = vec3(real(0), real(10e-15), real(0)) * real(m);
    e.position = vec3(real(10e-15), real(10e-15), real(0)) * real(m);

    vec3_t r = e.position - p.position;

    real vv = v(length(r));
    //    INFO("c = " + std::to_string(c));
    //    INFO("epsi0 = " + std::to_string(epsi0 * 1e20));
    //    INFO("e0 = " + std::to_string(e0 * 1e20));

    INFO("r = " + std::to_string(length(r)));
    INFO("vv = " + std::to_string(vv));
    INFO("time_step = " + std::to_string(time_step * 1e20));

    e.velocity = vv * normalize(cross(normalize(r), vec3(real(1))));

#undef v

    e.mass(m_e);
    e.charge = -e0;
    e.radius = r_e;
    particles.push_back(e);

    p.velocity = vec3(real(0));
    p.mass(m_p);
    p.charge = e0;
    p.radius = r_p;
    particles.push_back(p);
}

void
Simulation::compute_acceleration(vec3_t *acceleration,
                                 const vec3_t *position,
                                 const vec3_t *velocity)
{

    for (size_t i = 0; i < particles.size(); ++i) {
        Particle a = particles[i];
        vec3_t E = vec3(real(0));
        vec3_t B = vec3(real(0));

        for (size_t j = 0; j < particles.size(); ++j) {
            if (i == j)
                continue;
            const Particle b = particles[j];

            vec3_t r = position[i] - position[j];
            real r2 = dot(r, r);
            vec3_t n = normalize(r);

            E += inverse(4 * math::PI * epsi0 * r2) * b.charge * n;
            //            B += mu0 * b.charge * inverse(4 * math::PI * r2) *
            //            cross(velocity[j], n);
        }

        acceleration[i] = a.inv_mass * a.charge * (E + cross(velocity[i], B));
    }
}

void
Simulation::simulate_frame()
{
    const real dt = time_step;
    const real dt2 = real(.5) * dt;

    vec3_t *accel = new vec3_t[particles.size()];
    vec3_t *accel1 = new vec3_t[particles.size()];
    vec3_t *pos = new vec3_t[particles.size()];
    vec3_t *vel = new vec3_t[particles.size()];

    compute_acceleration(accel, particles._position, particles._velocity);
    for (size_t i = 0; i < particles.size(); ++i) {
        pos[i] = particles._position[i] + dt2 * particles._velocity[i];
        vel[i] = particles._velocity[i] + dt2 * accel[i];
        accel[i] *= real(1) / real(6);
    }

    compute_acceleration(accel1, pos, vel);
    for (size_t i = 0; i < particles.size(); ++i) {
        pos[i] = particles._position[i] + dt2 * vel[i];
        vel[i] = particles._velocity[i] + dt2 * accel1[i];
        accel[i] += (real(2) / real(6)) * accel1[i];
    }

    compute_acceleration(accel1, pos, vel);
    for (size_t i = 0; i < particles.size(); ++i) {
        pos[i] = particles._position[i] + dt * vel[i];
        vel[i] = particles._velocity[i] + dt * accel1[i];
        accel[i] += (real(2) / real(6)) * accel1[i];
    }

    compute_acceleration(accel1, pos, vel);
    for (size_t i = 0; i < particles.size(); ++i) {
        vec3_t a = accel[i] + (real(1) / real(6)) * accel1[i];
        particles._position[i] +=
          dt * particles._velocity[i] + real(.5) * dt * dt * a;
        particles._velocity[i] += dt * a;
    }

    delete[] accel;
    delete[] accel1;
    delete[] pos;
    delete[] vel;
}

void
Simulation::extrapolate_particle(Particle &p, math::real interpolation)
{
    p.position += interpolation * time_step * p.velocity;
}
