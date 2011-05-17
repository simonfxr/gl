#include "ge/Engine.hpp"

int main(int argc, char *argv[]) {
    return ge::Engine().run(ge::EngineOpts().parseOpts(&argc, &argv));
}
