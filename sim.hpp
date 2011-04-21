#ifndef SIM_HPP
#define SIM_HPP

#include "math/vec3/type.hpp"
#include "glt/color.hpp"
#include "glt/Frame.hpp"
#include "glt/AABB.hpp"
#include "glt/ViewFrustum.hpp"

enum SphereState { Bouncing, Rolling };

struct SphereModel {
    glt::color color;
    float shininess;
};

struct Sphere {
    SphereState state;
    math::vec3_t v;
    float r;
    math::point3_t center;
    float m;
};

struct Game;

// implemented in sim-sfml.cpp
struct Renderer {
    Renderer(Game& _g) : game(_g) {}
    Game& game;

    const glt::ViewFrustum& frustum();

    void renderSphere(const Sphere& sphere, const SphereModel& model);
    void endRenderSpheres();
    void renderBox(const glt::AABB& box);
    void renderConnection(const math::point3_t& a, const math::point3_t& b);
};

struct World {
    glt::Frame& camera();

    uint32 numSpheres();

    bool init();

    bool render_by_distance; // try to render near object first
    uint32 solve_iterations;

    void moveCamera(const math::vec3_t& step, float r);
    void rotateCamera(float rotx, float roty);
    void spawnSphere(const Sphere& sphere, const SphereModel& model);
    void simulate(float dt);
    
    void render(Renderer& renderer, float dt);

    World();
    ~World();

private:
    struct Data;
    Data * const self;

    World(const World& _);
    World& operator =(const World& _);
};

#endif
