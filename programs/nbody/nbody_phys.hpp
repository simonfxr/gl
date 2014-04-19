#ifndef _NBODY_PHYS_HPP
#define _NBODY_PHYS_HPP

#include "math/vec3.hpp"

using namespace defs;
using namespace math;

struct Particle;
struct ParticleRef;
struct ParticleArray;
struct Simulation;


struct Particle {
    point3_t position;
    vec3_t velocity;
    real inv_mass; // allow for m = infinity -> m^-1 = 0
    real charge;

    real mass() const { return real(1) / inv_mass; }
    Particle& mass(real m) { inv_mass = real(1) / m; return *this; }
};

struct ParticleArray {
    size _n;
    defs::size _size;

    point3_t *_position;
    vec3_t *_velocity;
    real *_inv_mass;
    real *_charge;

    ParticleArray(defs::size);
    ~ParticleArray();

    Particle operator[](index) const;
    ParticleRef operator[](index);

    void push_back(const Particle&);
    void put(index i, const Particle&);

    defs::size size() const { return _n; }
};

struct ParticleRef {
    ParticleArray& array;
    index i;

    ParticleRef(ParticleArray&, defs::index);

    operator Particle() const;
    ParticleRef& operator =(const Particle&);
};

struct Simulation {
    
    real time_step;
    ParticleArray particles;

    Simulation(real time_step);
    ~Simulation();

    void init();

    void simulate_frame();
    void extrapolate_particle(Particle& p, real interpolation);
};

#endif
