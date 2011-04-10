#include "sim.hpp"

#include "defs.h"

#include "math/vec3.hpp"
#include "math/plane.hpp"

#include <iostream>
#include <vector>
#include <algorithm>

#include <cstdlib>

using namespace math;

struct Particle {
    point3_t pos;
    vec3_t   vel;
    float    invMass;
};

struct ParticleRef {
    uint16 index;
    ParticleRef() {}
    ParticleRef(uint16 i) : index(i) {}
};

struct SphereRef {
    uint16 index;
    SphereRef() {}
    SphereRef(uint16 i) : index(i) {}
};

struct SphereData {
    ParticleRef particle;
    float r;
};

struct Contact {
    direction3_t normal;
    float distance;
    float restitution;
    ParticleRef x, y;
};

// struct Spring {
//     float D;
//     float l0;
// };

// struct Connection {
//     SphereRef x;
//     point3_t anchor;
//     Spring spring;
// };


static const vec3_t GRAVITY = vec3(0.f, -9.81f, 0.f);

static const float DAMPING = 0.97f;

static const vec3_t ROOM_DIMENSIONS = vec3(40.f, 30.f, 40.f);

static float rand1() {
    return rand() * (1.f / RAND_MAX);
}

struct World::Data {
    std::vector<Particle> particles;
    std::vector<SphereData> spheres;
    std::vector<SphereModel> sphereModels;
    
    glt::Frame _camera;
    glt::AABB room;

    plane3_t walls[6];
    ParticleRef world; // stationary particle with infinite mass

    Particle& deref(ParticleRef ref);
    SphereData& deref(SphereRef ref);
    Sphere makeSphere(SphereRef ref);
    
    int32 collidesWall(const point3_t& center, float r, point3_t& out_collision);

    void generateContacts(std::vector<Contact>& contacts, float dt);
    void solveContacts(std::vector<Contact>& contacts, float dt);
    void integrate(float dt);
};

World::World()
    : self(new Data)
{}

World::~World() {
    delete self;
}

bool World::init() {
    const vec3_t dim = ROOM_DIMENSIONS * 0.5f;

    self->walls[0] = plane(vec3(+1.f, 0.f, 0.f), ROOM_DIMENSIONS.x * -0.5f); // left 
    self->walls[1] = plane(vec3(-1.f, 0.f, 0.f), ROOM_DIMENSIONS.x * -0.5f); // right
    self->walls[2] = plane(vec3(0.f, +1.f, 0.f), ROOM_DIMENSIONS.y * -0.5f); // bot  
    self->walls[3] = plane(vec3(0.f, -1.f, 0.f), ROOM_DIMENSIONS.y * -0.5f); // top  
    self->walls[4] = plane(vec3(0.f, 0.f, +1.f), ROOM_DIMENSIONS.z * -0.5f); // far      
    self->walls[5] = plane(vec3(0.f, 0.f, -1.f), ROOM_DIMENSIONS.z * -0.5f); // near

    Particle world;
    world.invMass = 0.f;
    world.pos = vec3(0.f);
    world.vel = vec3(0.f);

    self->particles.push_back(world);
    self->world.index = 0;

    self->room.corner_min = -dim;
    self->room.corner_max = dim;

    return true;
}

glt::Frame& World::camera() {
    return self->_camera;
}

uint32 World::numSpheres() {
    return self->spheres.size();
}

namespace {
std::ostream& operator <<(std::ostream& out, const vec4_t& a) {
    return out << "(" << a[0] << ", " << a[1] << ", " << a[2] << ", " << a[3] << ")";
}
}

void World::moveCamera(const vec3_t& step, float r) {
    glt::Frame new_cam = self->_camera;
    new_cam.translateLocal(step);

    point3_t coll_center;
    int32 hit = self->collidesWall(new_cam.origin, r, coll_center);
    if (hit >= 0)
        new_cam.origin = self->walls[hit].normal * r + coll_center;

    self->_camera.origin = new_cam.origin;
}

void World::rotateCamera(float rotx, float roty) {
    self->_camera.rotateLocal(roty, vec3(1.f, 0.f, 0.f));
    self->_camera.rotateWorld(rotx, vec3(0.f, 1.f, 0.f));
}

void World::simulate(float dt) {
    {
        std::vector<Contact> contacts;
        self->generateContacts(contacts, dt);
        self->solveContacts(contacts, dt);
    }
    
    self->integrate(dt);
}

void World::spawnSphere(const Sphere& s, const SphereModel& m) {

    // Spring spring;
    // spring.D = 100.f;
    // spring.l0 = 1.f;

    Particle p;
    p.invMass = recip(s.m);
    p.vel = s.v;
    p.pos = s.center;
    
    SphereData sd;
    sd.particle.index = self->particles.size();
    sd.r = s.r;

    self->particles.push_back(p);
    self->spheres.push_back(sd);
    self->sphereModels.push_back(m);

    // uint16 n = self->spheres.size();

    // if (rand1() >= 0.5f) {
    //     Connection con;
    //     con.x = n - 1;
    //     con.spring = spring;
    //     vec3_t sel = vec3(rand1(), 1.f, rand1()) - vec3(0.5f);
    //     sel *= sel;
    //     sel *= sel;
    //     sel.y = 0.5f;
    //     con.anchor = ROOM_DIMENSIONS * sel;
    //     self->connected.push_back(con);
    // }
}

struct SphereDistance {
    SphereRef sphere;
    float viewDist;

    static bool lessThan(const SphereDistance& d1, const SphereDistance& d2) {
        return d1.viewDist < d2.viewDist;
    }
    
} ATTRS(ATTR_ALIGNED(8));

void World::render(Renderer& renderer, float dt) {
    const uint32 n = self->spheres.size();

    if (render_by_distance) {
        const point3_t& cam = self->_camera.origin;
        std::vector<SphereDistance> spheres_ordered(n);

        for (uint32 i = 0; i < n; ++i) {
            const SphereData& s = self->spheres[i];
            const Particle& p = self->deref(s.particle);
            const point3_t pos = p.pos + p.vel * dt;
            
            SphereDistance d;
            d.sphere = SphereRef(i);
            d.viewDist = max(0.f, distanceSq(pos, cam) - s.r * s.r);
            spheres_ordered[i] = d;
        }

        std::sort(spheres_ordered.begin(), spheres_ordered.end(), SphereDistance::lessThan);

        for (uint32 i = 0; i < n; ++i) {
            const SphereDistance& d = spheres_ordered[i];
            Sphere s = self->makeSphere(d.sphere);
            s.center += s.v * dt;
            renderer.renderSphere(s, self->sphereModels[d.sphere.index]);
        }

    } else {
        for (uint32 i = 0; i < n; ++i) {
            Sphere s = self->makeSphere(SphereRef(i));
            s.center += s.v * dt;
            renderer.renderSphere(s, self->sphereModels[i]);
        }
    }

    // uint32 ncon = self->connected.size();
    // for (uint32 i = 0; i < ncon; ++i) {
    //     Connection& c = self->connected[i];
    //     Sphere& x = self->spheres[c.x];
    //     direction3_t r = directionFromTo(x.center, c.anchor);
    //     renderer.renderConnection(x.center + x.r * r, c.anchor);
    // }

    renderer.renderBox(self->room);
}

int32 World::Data::collidesWall(const point3_t& center, float r, point3_t& out_collision) {
    float dist_min = r + 1;
    int32 hit = -1;

    for (uint32 i = 0; i < 6; ++i) {
        float dist = distance(walls[i], center);
        if (dist < r && dist < dist_min) {
            dist_min = dist;
            hit = i;
            out_collision = projectOnto(walls[i], center);
        }
    }

    return hit;
}

Particle& World::Data::deref(ParticleRef ref) {
    return particles[ref.index];
}

SphereData& World::Data::deref(SphereRef ref) {
    return spheres[ref.index];
}

void World::Data::generateContacts(std::vector<Contact>& contacts, float dt) {
    uint32 n = spheres.size();
    for (uint32 i = 0; i < n; ++i) {
        const SphereData& s1 = spheres[i];
        const Particle& p1 = deref(s1.particle);
        const point3_t pos1 = p1.pos;

        point3_t coll_pos;
        int32 hit = collidesWall(pos1, s1.r, coll_pos);
        if (hit >= 0) {
            Contact con;
            con.x = s1.particle;
            con.y = world;
            con.normal = walls[hit].normal;
            point3_t nearest = projectOnto(walls[hit], p1.pos);
            float dist = distance(p1.pos, nearest);
            con.distance = dist - s1.r;
            contacts.push_back(con);
        }

        for (uint32 j = 0; j < i; ++j) {
            const SphereData s2 = spheres[j];
            const Particle& p2 = deref(s2.particle);
            const point3_t pos2 = p2.pos;

            float dist2 = distanceSq(pos1, pos2);
            if (dist2 < squared(s1.r + s2.r)) {
                Contact con;
                con.x = s1.particle;
                con.y = s2.particle;
                con.normal = directionFromTo(pos2, pos1);
                con.distance = sqrt(dist2) - s1.r - s2.r;
                contacts.push_back(con);
            }
        }
    }
}

void World::Data::solveContacts(std::vector<Contact>& contacts, float dt) {
    uint32 n = contacts.size();
    for (uint32 i = 0; i < n; ++i) {
        const Contact& con = contacts[i];
        Particle& p1 = deref(con.x);
        Particle& p2 = deref(con.y);

        const direction3_t n = con.normal;

        float relNv = dot(p2.vel - p1.vel, n);
        float remove = relNv - con.distance / dt;

        if (remove > 0) {
            float p = remove / (p1.invMass + p2.invMass);
            p1.vel += p * p1.invMass * n;
            p2.vel -= p * p2.invMass * n;
        }
    }
}

void World::Data::integrate(float dt) {
    const vec3_t v_grav = GRAVITY * dt;
    
    uint32 n = particles.size();
    for (uint32 i = 0; i < n; ++i) {
        Particle& p = particles[i];
        p.pos += p.vel * dt;
        if (p.invMass != 0.f)
            p.vel += v_grav;
    }
}

Sphere World::Data::makeSphere(SphereRef ref) {
    SphereData s = deref(ref);
    Particle p = deref(s.particle);
    Sphere sp;
    sp.center = p.pos;
    sp.v = p.vel;
    sp.m = recip(p.invMass);
    sp.r = s.r;
    sp.state = Bouncing;
    return sp;
}
