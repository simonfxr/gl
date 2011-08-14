#ifndef PH_WORLD_HPP
#define PH_WORLD_HPP

#include "ph/defs.hpp"

namespace ph {

struct World;

struct ObjectID {
private:
    uint32 id;
    ObjectID(uint32 _id) : id(_id) {}
    friend struct World;
};

struct World {

    World();
    ~World();

    bool init();

    void step(math::real delta);
    
    ObjectID spawn(const RigidBody&);
    void remove(ObjectID obj);
    bool deref(ObjectID, RigidBody *);
    
private:

    World(const World&);
    World& operator =(const World&);

    struct Data;
    friend struct Data;

    Data * self;
};

} // namespace ph

#endif
