#include "nbody_phys.hpp"

#include "err/err.hpp"

using namespace math;

inline constexpr real m = 1e15_r;
inline constexpr real sec = 1e12_r;
inline constexpr real kg = 1_r;
inline constexpr real As = 1_r;

inline constexpr real epsi0 =
  8.854187817e-12_r * (As * As * (sec / m) * (sec / m) / (kg * m));
// inline constexpr real c = real(2.99792458e8) * m / sec;

// inline constexpr real mu0 = inverse(c * c * epsi0) * (kg * m / (As * As));

inline constexpr real r_p = 0.84e-15_r * m;
inline constexpr real r_e = 2.8e-15_r * m;

inline constexpr real m_p = 1.672e-27_r * kg;
inline constexpr real m_e = 9.109382e-31_r * kg;

inline constexpr real e0 = 1.6e-19_r * As;

ParticleRef::ParticleRef(ParticleArray &a, uint32_t _i) : array(a), i(_i) {}

ParticleArray::ParticleArray(size_t s)
  : _n(0)
  , _size(s)
  , _position(s)
  , _velocity(s)
  , _inv_mass(s)
  , _charge(s)
  , _radius(s)
{}

Particle
ParticleArray::operator[](size_t i) const
{
    auto p = Particle{};
    p.position = _position[i];
    p.velocity = _velocity[i];
    p.inv_mass = _inv_mass[i];
    p.charge = _charge[i];
    p.radius = _radius[i];
    return p;
}

ParticleRef
ParticleArray::operator[](size_t i)
{
    return { *this, static_cast<uint32_t>(i) };
}

void
ParticleArray::push_back(const Particle &p)
{
    if (_n >= _size) {
        auto old_size = _size;
        _size = old_size < 4 ? 8 : old_size * 2;

        _position.resize(_size);
        _velocity.resize(_size);
        _inv_mass.resize(_size);
        _charge.resize(_size);
        _radius.resize(_size);
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

Simulation::~Simulation() = default;

void
Simulation::init()
{

    /*
     * m_e v^2 = e^2 / (4 pi epsi0) 1/r
     * v = math::sqrt(e^2 / (4 pi epsi0 m_red) 1/r)
     */

#define v(r) math::sqrt(real(e0 *e0 / (4.0 * math::PI * epsi0 * m_e)) * (r))

    auto e = Particle{};
    auto p = Particle{};

    p.position = vec3(0_r, 10e-15_r, 0_r) * m;
    e.position = vec3(10e-15_r, 10e-15_r, 0_r) * m;

    auto r = e.position - p.position;

    auto vv = v(length(r));
    //    INFO("c = " + std::to_string(c));
    //    INFO("epsi0 = " + std::to_string(epsi0 * 1e20));
    //    INFO("e0 = " + std::to_string(e0 * 1e20));

    INFO("r = " + std::to_string(length(r)));
    INFO("vv = " + std::to_string(vv));
    INFO("time_step = " + std::to_string(time_step * 1e20));

    e.velocity = vv * normalize(cross(normalize(r), vec3(1_r)));

#undef v

    e.mass(m_e);
    e.charge = -e0;
    e.radius = r_e;
    particles.push_back(e);

    p.velocity = vec3(0_r);
    p.mass(m_p);
    p.charge = e0;
    p.radius = r_p;
    particles.push_back(p);
}

void
Simulation::compute_acceleration(vec3 *acceleration,
                                 const vec3 *position,
                                 const vec3 *velocity)
{

    for (size_t i = 0; i < particles.size(); ++i) {
        const Particle a = particles[i];
        auto E = vec3(0_r);
        auto B = vec3(0_r);

        for (size_t j = 0; j < particles.size(); ++j) {
            if (i == j)
                continue;
            const Particle b = particles[j];

            auto r = position[i] - position[j];
            auto r2 = dot(r, r);
            auto n = normalize(r);

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
    const real dt2 = .5_r * dt;

    auto accel = Array<vec3>(particles.size());
    auto accel1 = Array<vec3>(particles.size());
    auto pos = Array<vec3>(particles.size());
    auto vel = Array<vec3>(particles.size());

    compute_acceleration(
      accel.begin(), particles._position.begin(), particles._velocity.begin());
    for (size_t i = 0; i < particles.size(); ++i) {
        pos[i] = particles._position[i] + dt2 * particles._velocity[i];
        vel[i] = particles._velocity[i] + dt2 * accel[i];
        accel[i] *= 1_r / 6_r;
    }

    compute_acceleration(accel1.begin(), pos.begin(), vel.begin());
    for (size_t i = 0; i < particles.size(); ++i) {
        pos[i] = particles._position[i] + dt2 * vel[i];
        vel[i] = particles._velocity[i] + dt2 * accel1[i];
        accel[i] += (2_r / 6_r) * accel1[i];
    }

    compute_acceleration(accel1.begin(), pos.begin(), vel.begin());
    for (size_t i = 0; i < particles.size(); ++i) {
        pos[i] = particles._position[i] + dt * vel[i];
        vel[i] = particles._velocity[i] + dt * accel1[i];
        accel[i] += (2_r / 6_r) * accel1[i];
    }

    compute_acceleration(accel1.begin(), pos.begin(), vel.begin());
    for (size_t i = 0; i < particles.size(); ++i) {
        auto a = accel[i] + (1_r / 6_r) * accel1[i];
        particles._position[i] +=
          dt * particles._velocity[i] + .5_r * dt * dt * a;
        particles._velocity[i] += dt * a;
    }
}

void
Simulation::extrapolate_particle(Particle &p, math::real interpolation) const
{
    p.position += interpolation * time_step * p.velocity;
}
