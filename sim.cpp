#include "sim.hpp"

#include "defs.h"

#include "math/vec3.hpp"
#include "math/plane.hpp"

#include <iostream>
#include <vector>
#include <algorithm>

#include <cstdlib>
#include <ctime>

#include <SFML/System/Clock.hpp>

#if 0

#define time_msg(msg, op) do {                                          \
        sf::Clock c;                                                    \
        float __t0 = c.GetElapsedTime();                                \
        { op }                                                          \
        float __delta = c.GetElapsedTime() - __t0;                      \
        __delta *= 1000.f;                                              \
        std::cout << msg << " took " << __delta << " ms" << std::endl; \
    } while (0)

#else

#define time_msg(msg, op) do { op } while (0)

#endif

using namespace math;

struct Particle {
    point3_t pos;
    vec3_t   vel;
    float    invMass;
};

struct ParticleRef {
    uint16 index;
};

struct SphereRef {
    uint16 index;
};

namespace {

ParticleRef particleRef(uint16 index) {
    ParticleRef r; r.index = index; return r;
}

SphereRef sphereRef(uint16 index) {
    SphereRef r; r.index = index; return r;
}

}

struct SphereData {
    ParticleRef particle;
    float r;
};

struct Contact {
    direction3_t normal;
    float distance;
    float restitution;
    float impulse;
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

struct BSP;
struct Box;
struct Node;

struct World::Data {
    std::vector<Particle> particles;
    std::vector<SphereData> spheres;
    std::vector<SphereModel> sphereModels;
    
    glt::Frame _camera;
    glt::AABB room;

    uint32 solve_iterations;

    plane3_t walls[6];
    ParticleRef world; // stationary particle with infinite mass

    Particle& deref(ParticleRef ref);
    SphereData& deref(SphereRef ref);
    Sphere makeSphere(SphereRef ref);
    
    int32 collidesWall(const point3_t& center, float r, point3_t& out_collision);

    void generateContacts(std::vector<Contact>& contacts, float dt);
    void solveContacts(std::vector<Contact>& contacts, float dt);
    void integrate(float dt);

    void insertBSP(BSP *&p, int ni, Box volume, uint32 d, Node *node, const point3_t& center, float r);
    void findCollisions(std::vector<Contact>& contacts, const BSP *t, int ni, std::vector<bool>& collided, SphereRef s, const point3_t& center, float r);
};

World::World() :
    solve_iterations(1),

    self(new Data)
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
    self->world = particleRef(0);

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
        self->solve_iterations = solve_iterations;
        std::vector<Contact> contacts;
        time_msg("generating contacts", self->generateContacts(contacts, dt););
        time_msg("solving contacts", self->solveContacts(contacts, dt););
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
        std::vector<SphereDistance> spheres_ordered;

        for (uint32 i = 0; i < n; ++i) {
            const SphereData& s = self->spheres[i];
            const Particle& p = self->deref(s.particle);
            const point3_t pos = p.pos + p.vel * dt;

            if (glt::clipped(testSphere(renderer.frustum(), pos, s.r))) {
                continue;
            }
            
            SphereDistance d;
            d.sphere = sphereRef(i);
            d.viewDist = max(0.f, distanceSq(pos, cam) - s.r * s.r);
            //   spheres_ordered[i] = d;
            spheres_ordered.push_back(d);
        }

        for (uint32 i = 0; i < spheres_ordered.size(); ++i) {
            const SphereDistance& d = spheres_ordered[i];
            Sphere s = self->makeSphere(d.sphere);
            s.center += s.v * dt;
            renderer.renderSphere(s, self->sphereModels[d.sphere.index]);
        }

    } else {
        for (uint32 i = 0; i < n; ++i) {
            Sphere s = self->makeSphere(sphereRef(i));
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

struct Box {
    point3_t center;
    vec3_t halfWidth;
};

struct BSP;

bool isLeaf(const BSP* bsp);

struct Node {
    SphereRef sphere;
    Node *nxt;
};

struct BSP {
    plane3_t plane;

    union Child {
        BSP *tree;
        Node *list;
    };

    Child down[2]; // front, back

    ~BSP() {
        if (!isLeaf(this)) {
            delete down[0].tree;
            delete down[1].tree;
        } else {
            Node *node = down[1].list;
            while (node != 0) {
                Node *prev = node;
                node = node->nxt;
                prev->nxt = 0;
                delete prev;
            }
        }
    }
};

static const BSP marker_leaf = { plane(), { { 0 }, { 0 } } };
static BSP * const leaf_marker = (BSP *) &marker_leaf;


bool isLeaf(const BSP* t) {
    return t->down[0].tree == leaf_marker;
}

direction3_t NORMALS[3] = {
    vec3(1.f, 0.f, 0.f),
    vec3(0.f, 1.f, 0.f),
    vec3(0.f, 0.f, 1.f)
};

plane3_t makePlane(int ni, const Box& vol) {
    const vec3_t& n = NORMALS[ni];
    return plane(n, sum(vol.center * n));
}

Box split(const Box& enclosing, int ni, float sign) {
    Box b;
    b.halfWidth = enclosing.halfWidth * (vec3(1.f) - vec3(0.5f) * NORMALS[ni]);
    b.center = enclosing.center + sign * (enclosing.halfWidth - b.halfWidth);
    return b;
}

static const uint32 MAX_DEPTH = 10;

static uint32 num_tree;
static uint32 num_nodes;

void World::Data::insertBSP(BSP *&p, int ni, Box volume, uint32 depth, Node *node, const point3_t& center, float r) {
    
    if (p == 0) {
        p = new BSP;
        ++num_tree;
//        ASSERT(node->nxt == 0);
        p->plane = makePlane(ni, volume);
        p->down[0].tree = leaf_marker;
        p->down[1].list = node;
        return;
    }

    if (isLeaf(p)) {
        if (depth >= MAX_DEPTH) {
            node->nxt = p->down[1].list;
            p->down[1].list = node;
        } else {
            Node *other = p->down[1].list;
            // must have exactly one Node
//            ASSERT(other->nxt == 0);
            SphereRef s2 = other->sphere;
            const SphereData& s2d = deref(s2);
            const Particle& p2 = deref(s2d.particle);
            p->down[0].tree = p->down[1].tree = 0;
            insertBSP(p, ni, volume, depth, other, p2.pos, s2d.r);
            insertBSP(p, ni, volume, depth, node, center, r);
        }
        return;
    }

    float d = distance(p->plane, center);
    float d0 = abs(d);
    float sign = signum(d);
    uint32 ni2 = (ni + 1) % 3;
    if (sign == 0.f) sign = 1.f;
    uint32 face = sign > 0 ? 0 : 1;
    
    ++depth;

    if (d0 > r) {
        insertBSP(p->down[face].tree, ni2, split(volume, ni2, sign), depth, node, center, r);
        return;
    }

    insertBSP(p->down[face].tree, ni2, split(volume, ni2, sign), depth, node, center, r);
    Node *copy = new Node;
    ++num_nodes;
    copy->nxt = 0;
    copy->sphere = node->sphere;
    insertBSP(p->down[!face].tree, ni2, split(volume, ni2, -sign), depth, copy, center, r);
}

void World::Data::findCollisions(std::vector<Contact>& contacts, const BSP *t, int ni, std::vector<bool>& collided,SphereRef s, const point3_t& center, float r) {
    
    if (t == 0) return;
    
    if (isLeaf(t)) {

        Node *list = t->down[1].list;
        while (list != 0) {

            if (s.index > list->sphere.index) {
                const SphereData& s2 = deref(list->sphere);
                const Particle& p2 = deref(s2.particle);

                if (!collided[s2.particle.index]) {
                    float dist2 = distanceSq(center, p2.pos);
                    if (dist2 < squared(r + s2.r)) {
                        collided[s2.particle.index] = true;
                        Contact con;
                        const SphereData& s1 = deref(s);
                        con.x = s1.particle;
                        con.y = s2.particle;
                        con.normal = directionFromTo(p2.pos, center);
                        con.distance = sqrt(dist2) - r - s2.r;
                        contacts.push_back(con);
                    }
                }
            }
            
            list = list->nxt;
        }
        
        return;
    }

    float d = distance(t->plane, center);
    float d0 = abs(d);
    float sign = signum(d);
    uint32 ni2 = (ni + 1) % 3;
    if (sign == 0.f) sign = 1.f;
    uint32 face = sign > 0 ? 0 : 1;

    if (d0 > r) {
        findCollisions(contacts, t->down[face].tree, ni2, collided, s, center, r);
        return;
    }

    findCollisions(contacts, t->down[face].tree, ni2, collided, s, center, r);
    findCollisions(contacts, t->down[!face].tree, ni2, collided, s, center, r);
}

void World::Data::generateContacts(std::vector<Contact>& contacts, float dt) {

    UNUSED(dt);
    
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
            con.impulse = 0.f;
            point3_t nearest = projectOnto(walls[hit], p1.pos);
            float dist = distance(p1.pos, nearest);
            con.distance = dist - s1.r;
            contacts.push_back(con);
        }

        // for (uint32 j = 0; j < i; ++j) {
        //     const SphereData s2 = spheres[j];
        //     const Particle& p2 = deref(s2.particle);
        //     const point3_t pos2 = p2.pos;

        //     float dist2 = distanceSq(pos1, pos2);
        //     if (dist2 < squared(s1.r + s2.r)) {
        //         Contact con;
        //         con.x = s1.particle;
        //         con.y = s2.particle;
        //         con.normal = directionFromTo(pos2, pos1);
        //         con.distance = sqrt(dist2) - s1.r - s2.r;
        //         contacts.push_back(con);
        //     }
        // }
    }

    BSP *root = 0;
    Box volume;
    volume.center = room.center();
    volume.halfWidth = 0.5f * (room.corner_max - room.corner_min);

    num_tree = 0;
    num_nodes = 0;

    time_msg("tree building", 
    for (uint32 i = 0; i < n; ++i) {
        const SphereData& s1 = spheres[i];
        const Particle& p1 = deref(s1.particle);
        Node *node = new Node;
        ++num_nodes;
        node->nxt = 0;
        node->sphere = sphereRef(i);
        insertBSP(root, 0, volume, 0, node, p1.pos, s1.r);
    });

    time_msg("testing collisions",
    std::vector<bool> collided(particles.size(), false);

    for (uint32 i = 0; i < n; ++i) {
        const SphereData& s1 = spheres[i];
        const Particle& p1 = deref(s1.particle);
        uint32 num_coll = contacts.size();
        findCollisions(contacts, root, 0, collided, sphereRef(i), p1.pos, s1.r);
        uint32 k = contacts.size();
        for (uint32 j = num_coll; j < k; ++j) {
            collided[contacts[j].y.index] = false;
        }
    });

//    std::cerr << "number of tree nodes: " << num_tree << ", number of list nodes: " << num_nodes << std::endl;

    delete root;            
}

void World::Data::solveContacts(std::vector<Contact>& contacts, float dt) {
    const uint32 n = contacts.size();

    for (uint32 k = 0; k < solve_iterations; ++k) {
        
        for (uint32 i = 0; i < n; ++i) {
            // const Contact& con = contacts[i];
            // Particle& p1 = deref(con.x);
            // Particle& p2 = deref(con.y);

            // const direction3_t n = con.normal;

            // float relNv = dot(p2.vel - p1.vel, n);
            // float remove = relNv - con.distance / dt;

            // if (remove > 0) {
            //     float p = remove / (p1.invMass + p2.invMass);
            //     p1.vel += p * p1.invMass * n;
            //     p2.vel -= p * p2.invMass * n;
            // }

            Contact& con = contacts[i];

            const vec3_t& n = con.normal;

            Particle& p1 = deref(con.x);
            Particle& p2 = deref(con.y);

// get all of relative normal velocity
            float relNv = dot(p2.vel - p1.vel, n);
 
// we want to remove only the amount which leaves them touching next frame
            float remove = relNv - con.distance / dt;
 
// compute impulse
            float imp = remove / (p1.invMass + p2.invMass);
 
// accumulate
            float newImpulse = max(imp + con.impulse, 0.f);
 
// compute change
            float change = newImpulse - con.impulse;
 
// store accumulated impulse
            con.impulse = newImpulse;
 
// apply impulse
            p1.vel += change * n * p1.invMass;
            p2.vel -= change * n * p2.invMass;
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
