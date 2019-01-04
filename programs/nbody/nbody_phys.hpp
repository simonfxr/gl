#ifndef _NBODY_PHYS_HPP
#define _NBODY_PHYS_HPP

#include "math/vec3.hpp"

using namespace math;

struct Particle;
struct ParticleRef;
struct ParticleArray;
struct Simulation;

struct Particle
{
    point3_t position;
    vec3_t velocity;
    real inv_mass; // allow for m = infinity -> m^-1 = 0
    real charge;
    real radius;

    real mass() const { return real(1) / inv_mass; }
    Particle &mass(real m)
    {
        inv_mass = real(1) / m;
        return *this;
    }
};

struct ParticleArray
{
    size_t _n;
    size_t _size;

    point3_t *_position;
    vec3_t *_velocity;
    real *_inv_mass;
    real *_charge;
    real *_radius;

    ParticleArray(size_t);
    ~ParticleArray();

    Particle operator[](size_t) const;
    ParticleRef operator[](size_t);

    void push_back(const Particle &);
    void put(size_t i, const Particle &);

    size_t size() { return _n; }
};

struct ParticleRef
{
    ParticleArray &array;
    uint32_t i;

    ParticleRef(ParticleArray &, uint32_t);

    operator Particle() const;
    ParticleRef &operator=(const Particle &);
};

struct Simulation
{

    real time_step;
    ParticleArray particles;

    Simulation(real time_step);
    ~Simulation();

    void init();

    void simulate_frame();
    void extrapolate_particle(Particle &p, real interpolation);

    void compute_acceleration(vec3_t *acceleration,
                              const vec3_t *position,
                              const vec3_t *velocity);
};

#endif
