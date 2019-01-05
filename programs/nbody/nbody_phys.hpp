#ifndef NBODY_PHYS_HPP
#define NBODY_PHYS_HPP

#include "math/vec3.hpp"

struct Particle;
struct ParticleRef;
struct ParticleArray;
struct Simulation;

struct Particle
{
    math::point3_t position;
    math::vec3_t velocity;
    math::real inv_mass; // allow for m = infinity -> m^-1 = 0
    math::real charge;
    math::real radius;

    math::real mass() const { return math::real(1) / inv_mass; }
    Particle &mass(math::real m)
    {
        inv_mass = math::real(1) / m;
        return *this;
    }
};

struct ParticleArray
{
    size_t _n;
    size_t _size;

    math::point3_t *_position;
    math::vec3_t *_velocity;
    math::real *_inv_mass;
    math::real *_charge;
    math::real *_radius;

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

    math::real time_step;
    ParticleArray particles;

    Simulation(math::real time_step);
    ~Simulation();

    void init();

    void simulate_frame();
    void extrapolate_particle(Particle &p, math::real interpolation);

    void compute_acceleration(math::vec3_t *acceleration,
                              const math::vec3_t *position,
                              const math::vec3_t *velocity);
};

#endif
