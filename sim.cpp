#include "sim.hpp"

#include "defs.h"

#include "math/vec3.hpp"
#include "math/plane.hpp"

#include <iostream>
#include <vector>
#include <algorithm>

#include <cstdlib>

using namespace math;

struct Spring {
    float D;
    float l0;
};

struct Connection {
    uint32 id;
    point3_t anchor;
    Spring spring;
};

static const vec3_t GRAVITY = vec3(0.f, -9.81f, 0.f);

static const float DAMPENING = 0.99f;

static const vec3_t ROOM_DIMENSIONS = vec3(40.f, 30.f, 40.f);

static float rand1() {
    return rand() * (1.f / RAND_MAX);
}

const plane3_t WALLS[6] = {
    plane(vec3(+1.f, 0.f, 0.f), ROOM_DIMENSIONS.x * -0.5f), // left 
    plane(vec3(-1.f, 0.f, 0.f), ROOM_DIMENSIONS.x * -0.5f), // right
    plane(vec3(0.f, +1.f, 0.f), ROOM_DIMENSIONS.y * -0.5f), // bot  
    plane(vec3(0.f, -1.f, 0.f), ROOM_DIMENSIONS.y * -0.5f), // top  
    plane(vec3(0.f, 0.f, +1.f), ROOM_DIMENSIONS.z * -0.5f), // far      
    plane(vec3(0.f, 0.f, -1.f), ROOM_DIMENSIONS.z * -0.5f), // near 
};

struct World::Data {
    std::vector<Sphere> spheres;
    std::vector<SphereModel> sphereModels;
    std::vector<Connection> connected;
    
    glt::Frame _camera;
    glt::AABB room;

    bool collidesWall(const point3_t& center, float r, direction3_t& out_norma, point3_t& out_collision) ATTRS(ATTR_NOINLINE);

    point3_t spherePosition(const Sphere& s, float dt) const;
    bool collideSpheres(Sphere& x, Sphere& y, float dt);
    void moveSphere(Sphere& s, float dt, float damp);
};

World::World()
    : self(new Data)
{}

World::~World() {
    delete self;
}

bool World::init() {
    const vec3_t dim = ROOM_DIMENSIONS * 0.5f;

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

    direction3_t coll_norm;
    point3_t coll_center;
    if (self->collidesWall(new_cam.origin, r, coll_norm, coll_center))
        new_cam.origin = coll_norm * r + coll_center;

    self->_camera.origin = new_cam.origin;
}

void World::rotateCamera(float rotx, float roty) {
    self->_camera.rotateLocal(roty, vec3(1.f, 0.f, 0.f));
    self->_camera.rotateWorld(rotx, vec3(0.f, 1.f, 0.f));
}

void World::simulate(float dt) {
    const uint32 n = self->spheres.size();
    
    for (uint32 i = 0; i < n; ++i)
        for (uint32 j = 0; j < i; ++j)
            self->collideSpheres(self->spheres[i], self->spheres[j], dt);

    float damp = pow(DAMPENING, dt);

    for (uint32 i = 0; i < n; ++i)
        self->moveSphere(self->spheres[i], dt, damp);

    for (uint32 i = 0; i < self->connected.size(); ++i) {
        Connection& con = self->connected[i];
        
        Sphere& x = self->spheres[con.id];
        point3_t anchor = con.anchor;

        // F = -D (l - l0) (negative for the pull)
        // p = F dt
        // v = v0 + p / m
        float l = distance(x.center, con.anchor);
        float F = - con.spring.D * (l - con.spring.l0);
        direction3_t r0 = (x.center - con.anchor) / l;
        x.v += (F * dt / x.m) * r0;
    }
}

void World::spawnSphere(const Sphere& s, const SphereModel& m) {

    Spring spring;
    spring.D = 100.f;
    spring.l0 = 1.f;

    self->spheres.push_back(s);
    self->sphereModels.push_back(m);

    uint16 n = self->spheres.size();

    if (rand1() >= 0.5f) {
        Connection con;
        con.id = n - 1;
        con.spring = spring;
        con.anchor = ROOM_DIMENSIONS * (vec3(rand1(), 1.f, rand1()) - vec3(0.5f));
        self->connected.push_back(con);
    }
}

struct SphereDistance {
    uint32 id;
    float viewDist;
    point3_t renderPos;

    static bool lessThan(const SphereDistance& d1, const SphereDistance& d2) {
        return d1.viewDist < d2.viewDist;
    }
};

void World::render(Renderer& renderer, float dt) {
    const uint32 n = self->spheres.size();

    if (render_by_distance) {
        const point3_t& cam = self->_camera.origin;
        std::vector<SphereDistance> spheres_ordered(n);

        for (uint32 i = 0; i < n; ++i) {
            Sphere& s = self->spheres[i];
            SphereDistance d;
            d.id = i;
            d.renderPos = self->spherePosition(s, dt);
            d.viewDist = max(0.f, distanceSq(d.renderPos, cam) - s.r * s.r);
            spheres_ordered[i] = d;
        }

        std::sort(spheres_ordered.begin(), spheres_ordered.end(), SphereDistance::lessThan);

        for (uint32 i = 0; i < n; ++i) {
            const SphereDistance& d = spheres_ordered[i];
            Sphere& s = self->spheres[d.id];
            point3_t old_center = s.center;
            s.center = d.renderPos;
            renderer.renderSphere(s, self->sphereModels[d.id]);
            s.center = old_center;
        }

    } else {
        for (uint32 i = 0; i < n; ++i)
            renderer.renderSphere(self->spheres[i], self->sphereModels[i]);
    }

    uint32 ncon = self->connected.size();
    for (uint32 i = 0; i < ncon; ++i) {
        Connection& c = self->connected[i];
        Sphere& x = self->spheres[c.id];
        direction3_t r = directionFromTo(x.center, c.anchor);
        renderer.renderConnection(x.center + x.r * r, c.anchor);
    }

    renderer.renderBox(self->room);
}

bool World::Data::collidesWall(const point3_t& center, float r, direction3_t& out_normal, point3_t& out_collision) {
    float dist_min = r + 1;

    for (uint32 i = 0; i < 6; ++i) {
        float dist = distance(WALLS[i], center);
        if (dist < r && dist < dist_min) {
            dist_min = dist;
            out_normal = WALLS[i].normal;
            out_collision = projectOnto(WALLS[i], center);
        }
    }

    return dist_min < r;
}

void World::Data::moveSphere(Sphere& s, float dt, float damp) {
    s.center = spherePosition(s, dt);

    vec3_t a_friction = vec3(0.f);
    // if (s.state == Rolling) {
    //     // Fr = mu FN
    //     // FN = -Fg
    //     // Fg = m g
    //     // Fr = - mu m g
    //     // a  = Fr / m
    //     // a  = - mu m g / m
    //     //    = - mu g
        
    //     float mu = 0.01;
    //     vec3_t a_friction = - length(GRAVITY) * mu * normalize(s.v);
    // }
    
    s.v += (GRAVITY + a_friction) * dt;
    s.v *= damp;
    
    direction3_t coll_norm;
    point3_t coll_pos;

    if (collidesWall(s.center, s.r, coll_norm, coll_pos)) {
        s.v = reflect(s.v, coll_norm, rand1() * 0.1f + 0.9f) * 0.8f;
        s.center = coll_pos + coll_norm * s.r;

        if (equal(coll_norm, vec3(0.f, 1.f, 0.f)) && dot(s.v, coll_norm) < 0.2f) {
            s.state = Rolling;
            s.v -= dot(s.v, coll_norm) * coll_norm;
        }
    }
}

point3_t World::Data::spherePosition(const Sphere& s, float dt) const {
    // s = s0 + v0 * dt + 1/2 * a * dt^2
    // (dt^2 is really small so we can ignore the quadratic term)
    return s.center + s.v * dt;
}

bool World::Data::collideSpheres(Sphere& x, Sphere& y, float dt) {
    point3_t xc = spherePosition(x, dt);
    point3_t yc = spherePosition(y, dt);

    float r = x.r + y.r;
    
    if (distanceSq(xc, yc) < r*r) {

        // HACK: if we collided in the last frame and collision wasnt resolved correctly
        // do it now, with a crude approximation (well no approx. at all...)
        if (distanceSq(x.center, y.center) < r*r) {

            vec3_t n1 = normalize(y.center - x.center);

            x.v = length(x.v) * -n1;
            y.v = length(y.v) * n1;

        } else {

            direction3_t toY = directionFromTo(xc, yc);
            direction3_t toX = -toY;
            
            vec3_t vx_coll = projectAlong(x.v, toY);
            vec3_t vx_rest = x.v - vx_coll;

            vec3_t vy_coll = projectAlong(y.v, toX);
            vec3_t vy_rest = y.v - vy_coll;

            vec3_t ux = (x.m * vx_coll + y.m * (2.f * vy_coll - vx_coll)) / (x.m + y.m);
            
            // conservation of momentum
            vec3_t uy = (x.m / y.m) * (vx_coll - ux) + vy_coll;

            x.v = (rand1() * 0.1f + 0.9f) * ux + vx_rest;
            y.v = (rand1() * 0.1f + 0.9f) * uy + vy_rest;
        }

        return true;
    }

    return false;
}

// namespace {

// static const uint32 ALLOC_BLOCK_SIZE = 1024;
// static const uint32 DIM = 50;

// struct SphereCollisionHandler {

//     struct Tile {
//         uint32 id;
//         Tile *nxt;
//     };

//     struct AllocBlock {
//         AllocBlock *nxt;
//         Tile block[ALLOC_BLOCK_SIZE];

//         AllocBlock(AllocBlock *pred = 0) :
//             nxt(pred)
//             {}
//     };

//     Tile *tiles[DIM][DIM][DIM];
    
//     std::vector<uint32> last_collisions;
//     uint32 next_tile;
//     AllocBlock *blocks;

//     SphereCollisionHandler(uint32 nspheres) :
//         last_collisions(nspheres, 0),
//         next_tile(0),
//         blocks(new AllocBlock)
//     {
//         memset(tiles, 0, DIM * DIM * DIM * sizeof tiles[0][0][0]);
//     }

//     ~SphereCollisionHandler() {
//         while (blocks != 0) {
//             AllocBlock *b = blocks;
//             blocks = blocks->nxt;
//             delete b;
//         }            
//     }

//     void insertTile(uint32 x, uint32 y, uint32 z, uint32 id) {
        
//         if (unlikely(next_tile >= ALLOC_BLOCK_SIZE)) {
//             next_tile = 0;
//             blocks = new AllocBlock(blocks);
//         }

//         Tile *t = &blocks->block[next_tile++];

//         t->id = id;
//         t->nxt = tiles[x][y][z];
//         tiles[x][y][z] = t;
//     }

//     void collTile(std::vector<Sphere>& ss, float dt, uint32 x, uint32 y, uint32 z, uint32 id) {
//         Sphere &s = ss[id];
//         Tile *t = tiles[x][y][z];
        
//         while (t != 0) {
            
//             if (t->id < id && last_collisions[id] <= t->id) {
//                 Sphere::collide(s, ss[t->id], dt);
//                 last_collisions[id] = t->id + 1;
//             }
            
//             t = t->nxt;
//         }
//     }
    
// };

// } // namespace anon

// static ivec3 calcTile(const vec3_t& isize, const vec3_t& room_min, const vec3_t& p) {
//     return ivec3(compMult(isize, p - room_min)); 
// }

// static void calcRange(const Sphere& s, float dt, const vec3_t& isize, const vec3_t& room_min, ivec3& tileL, ivec3& tileH) {

//     vec3_t pos1 = s.center;
//     vec3_t pos2 = s.calc_position(dt);

//     vec3_t cLow  = min(pos1, pos2) - vec3(s.r * 1.1f);
//     vec3_t cHigh = max(pos1, pos2) + vec3(s.r * 1.1f);

//     tileL = calcTile(isize, room_min, cLow);
//     tileH = calcTile(isize, room_min, cHigh);
    
// }

// static ivec3 clamp(const ivec3& low, const ivec3& high, const ivec3& v) {
//     return imax(low, imin(high, v));
// }

// static void clampRange(const ivec3& low, const ivec3& high, ivec3& range_low, ivec3& range_high) {
//     range_low = clamp(low, high, range_low);
//     range_high = clamp(low, high, range_high);
// }

// void Game::resolve_collisions(float dt) {

//     // float t0 = now();

//     // {
//     //     const uint32 dim = 50;
//     //     const vec3_t diff = room.corner_max - room.corner_min;
//     //     const vec3_t isize = vec3(dim / diff.x, dim / diff.y, dim / diff.z);
//     //     const vec3_t low  = room.corner_min;

//     //     const ivec3 tmin = ivec3(0);
//     //     const ivec3 tmax = ivec3(dim - 1);

//     //     SphereCollisionHandler coll(spheres.size());

//     //     for (uint32 i = 0; i < spheres.size(); ++i) {
        
//     //         Sphere& s = spheres[i];

//     //         ivec3 tileL, tileH;
//     //         calcRange(s, dt, isize, room.corner_min, tileL, tileH);
//     //         clampRange(tmin, tmax, tileL, tileH);

//     //         for (int32 x = tileL.x; x <= tileH.x; ++x)
//     //             for (int32 y = tileL.y; y <= tileH.y; ++y)
//     //                 for (int32 z = tileL.z; z <= tileH.z; ++z)
//     //                     coll.insertTile(x, y, z, i);
//     //     }

//     //     for (unsigned i = 0; i < spheres.size(); ++i) {
        
//     //         Sphere& s = spheres[i];
//     //         ivec3 tileL, tileH;
//     //         calcRange(s, dt, isize, room.corner_min, tileL, tileH);
//     //         clampRange(tmin, tmax, tileL, tileH);

//     //         for (int32 x = tileL.x; x <= tileH.x; ++x)
//     //             for (int32 y = tileL.y; y <= tileH.y; ++y)
//     //                 for (int32 z = tileL.z; z <= tileH.z; ++z)
//     //                     coll.collTile(spheres, dt, x, y, z, i);
//     //     }
//     // }

//     // std::cerr << "resolve_collisions: " << (now() - t0) << " seconds" << std::endl;

//     for (uint32 i = 0; i < spheres.size(); ++i)
//         for (uint32 j = 0; j < i; ++j)
//             Sphere::collide(spheres[i], spheres[j], dt);
    
// }
