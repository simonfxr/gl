#include "ge/Engine.hpp"

#include <iostream>

using namespace ge;

int main(int argc, char *argv[]) {
    EngineOpts opts;
    opts.parseOpts(&argc, &argv);
    return Engine().run(opts);
}
