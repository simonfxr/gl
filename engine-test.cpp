#include "ge/Engine.hpp"

#include <iostream>

using namespace ge;

struct Obj {
    void init(const Event<InitEvent>& e) {
        e.info.success = true;
        std::cout << "hello from Obj" << std::endl;
    }
};

int main(int argc, char *argv[]) {
    EngineOpts opts;
    opts.parseOpts(&argc, &argv);
    Obj x;
    opts.inits.init.reg(makeEventHandler(&x, &Obj::init));
    return Engine().run(opts);
}
